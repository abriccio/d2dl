const std = @import("std");
const windows = std.os.windows;
const HWND = windows.HWND;
const wchar = windows.WCHAR;

const Gui = @import("../Gui.zig");

// (big ol') TODO: Use marlersoft/zigwin32 as the Windows layer, eliminate
// our C++ library. Then these structs below will just be aliases of the direct
// Win32 types, and we dont' have to deal with compiling C and keeping 1,000
// different API declarations in sync.

const Context = *opaque {};

const Color = extern struct {
    r: f32,
    g: f32,
    b: f32,
    a: f32 = 1,
};

const Point = extern struct {
    x: f32,
    y: f32,
};

const Size = extern struct {
    width: f32,
    height: f32,
};

const Rect = extern struct {
    left: f32,
    top: f32,
    right: f32,
    bottom: f32,
};

pub const Direct2D = struct {
    ctx: Context,

    pub const WindowType = union(enum) {
        desktop,
        child: struct { user: *Gui },
    };

    pub fn init(
        width: i32,
        height: i32,
        window_title: [:0]const u8,
        window_type: WindowType,
    ) !Direct2D {
        switch (window_type) {
            .desktop => return .{
                .ctx = d2dl_initDesktopWindow(width, height, window_title) orelse {
                    return error.WindowInitFailed;
                },
            },
            .child => |child| return .{
                .ctx = d2dl_initChildWindow(
                    child.user,
                    width,
                    height,
                    window_title,
                ) orelse return error.WindowInitFailed,
            },
        }
    }

    pub fn deinit(self: Direct2D) void {
        d2dl_deinit(self.ctx);
    }

    pub fn setParent(self: Direct2D, parent: HWND) void {
        d2dl_setParent(self.ctx, parent);
    }

    pub fn setVisible(self: Direct2D, visible: bool) void {
        d2dl_setVisible(self.ctx, visible);
    }

    pub fn beginDrawing(self: Direct2D) void {
        d2dl_beginDrawing(self.ctx);
    }

    pub fn endDrawing(self: Direct2D) void {
        d2dl_endDrawing(self.ctx);
    }

    pub fn clear(self: Direct2D, color: Color) void {
        d2dl_clear(self.ctx, color);
    }

    pub fn fillRect(self: Direct2D, rect: Rect, color: Color) void {
        d2dl_fillRect(self.ctx, rect, color);
    }
};

extern fn d2dl_initDesktopWindow(width: c_int, height: c_int, window_title: [*:0]const u8) ?Context;
extern fn d2dl_initChildWindow(user: *Gui, width: c_int, height: c_int, window_title: [*:0]const u8) ?Context;
extern fn d2dl_deinit(Context) void;
extern fn d2dl_beginDrawing(Context) void;
extern fn d2dl_endDrawing(Context) void;
extern fn d2dl_clear(Context, color: Color) void;
extern fn d2dl_drawRect(Context, rect: Rect, stroke_width: f32, color: Color) void;
extern fn d2dl_fillRect(Context, rect: Rect, color: Color) void;
extern fn d2dl_drawRoundedRect(Context, rect: Rect, radius: f32, stroke_width: f32, color: Color) void;
extern fn d2dl_fillRoundedRect(Context, rect: Rect, radius: f32, color: Color) void;
extern fn d2dl_drawEllipse(Context, center: Point, rx: f32, ry: f32, stroke_width: f32, color: Color) void;
extern fn d2dl_fillEllipse(Context, center: Point, rx: f32, ry: f32, color: Color) void;
extern fn d2dl_drawText(Context, text: [*]const wchar, rect: Rect, color: Color) void;
extern fn d2dl_getRenderSize(Context) Size;
extern fn d2dl_setRenderSize(Context, size: Size) void;
extern fn d2dl_loadFont(Context, font_name: [*]const wchar, size: f32) void;
extern fn d2dl_setParent(Context, hwnd: HWND) void;
extern fn d2dl_setVisible(Context, bool) void;
