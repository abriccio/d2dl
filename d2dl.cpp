#define WIN32_LEAN_AND_MEAN
#include "d2dl.h"
#include <d2d1.h>
#include <dwrite.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct D2DLContext {
    HWND window;
    ID2D1Factory *factory;
    ID2D1HwndRenderTarget *rt;
    IDWriteFactory *dwrite_factory;
    // TODO: Support loading an arbitrary number of these
    IDWriteTextFormat *text_format;
} D2DLContext;

static D2DPoint circle_pos = {0};
static D2DPoint circle_d = {10, 10};

static ID2D1SolidColorBrush *createBrushForColor(D2DLContext *ctx, D2DColor color) {
    ID2D1SolidColorBrush *brush = NULL;
    HRESULT hr = ctx->rt->CreateSolidColorBrush(
        {.r = color.r, .g = color.g, .b = color.b, .a = color.a},
        &brush
    );
    if (!SUCCEEDED(hr)) {
        log_err_return_null("Brush create failed");
    }

    return brush;
}

void d2dl_beginDrawing(D2DLContext *ctx) {
    ctx->rt->BeginDraw();
}

void d2dl_endDrawing(D2DLContext *ctx) {
    HRESULT hr = ctx->rt->EndDraw();
    if (!SUCCEEDED(hr)) {
        log_err_return("EndDraw failed for some reason");
    }
}

void d2dl_clear(D2DLContext *ctx, D2DColor color) {
    ctx->rt->Clear({.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void d2dl_drawRect(D2DLContext *ctx, D2DRect rect, float stroke_width, D2DColor color) {
    auto brush = createBrushForColor(ctx, color);
    ctx->rt->DrawRectangle(
        {.left = rect.left, .top = rect.top, .right = rect.right, .bottom = rect.bottom},
        brush,
        stroke_width
    );

    brush->Release();
}

void d2dl_fillRect(D2DLContext *ctx, D2DRect rect, D2DColor color) {
    auto brush = createBrushForColor(ctx, color);
    ctx->rt->FillRectangle(
        {.left = rect.left, .top = rect.top, .right = rect.right, .bottom = rect.bottom},
        brush
    );

    brush->Release();
}

void d2dl_drawRoundedRect(D2DLContext *ctx, D2DRect rect, float radius,
                          float stroke_width, D2DColor color) {
    auto brush = createBrushForColor(ctx, color);
    D2D1_RECT_F reg_rect = {.left = rect.left, .top = rect.top, .right = rect.right, .bottom = rect.bottom};
    ctx->rt->DrawRoundedRectangle(
        {.rect = reg_rect, .radiusX = radius, .radiusY = radius},
        brush, stroke_width
    );

    brush->Release();
}

void d2dl_fillRoundedRect(D2DLContext *ctx, D2DRect rect, float radius,
                          D2DColor color) {
    auto brush = createBrushForColor(ctx, color);
    D2D1_RECT_F reg_rect = {.left = rect.left, .top = rect.top, .right = rect.right, .bottom = rect.bottom};
    ctx->rt->FillRoundedRectangle(
        {.rect = reg_rect, .radiusX = radius, .radiusY = radius},
        brush
    );

    brush->Release();
}

void d2dl_drawEllipse(D2DLContext *ctx, D2DPoint center, float rx, float ry,
                      float stroke_width, D2DColor color) {
    auto brush = createBrushForColor(ctx, color);
    ctx->rt->DrawEllipse(
        {{center.x, center.y}, rx, ry},
        brush,
        stroke_width
    );

    brush->Release();
}

void d2dl_fillEllipse(D2DLContext *ctx, D2DPoint center, float rx, float ry,
                      D2DColor color) {
    auto brush = createBrushForColor(ctx, color);
    ctx->rt->FillEllipse(
        {{center.x, center.y}, rx, ry},
        brush
    );

    brush->Release();
}

void d2dl_drawText(D2DLContext *ctx, const wchar_t *text, D2DRect rect, D2DColor color) {
    auto brush = createBrushForColor(ctx, color);
    ctx->text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    ctx->text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    size_t len = wcslen(text);
    ctx->rt->DrawText(text, len, ctx->text_format,
        {.left = rect.left, .top = rect.top, .right = rect.right, .bottom = rect.bottom},
        brush
    );

    brush->Release();
}

D2DSize d2dl_getRenderSize(D2DLContext *ctx) {
    auto size = ctx->rt->GetSize();
    return {.width = size.width, .height = size.height};
}

void d2dl_setRenderSize(D2DLContext *ctx, D2DSize size) {
    HRESULT hr = ctx->rt->Resize({.width = (u32)size.width, .height = (u32)size.height});
    if (!SUCCEEDED(hr)) {
        log_err_return("Resize failed");
    }
}

static void d2dl_createFactory(D2DLContext *ctx) {
  HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                 &ctx->factory);
  if (SUCCEEDED(hr)) {
    return;
  } else {
    log_err_return("ID2D1Factory creation failed");
  }
}

static void d2dl_createDirectWriteFactory(D2DLContext *ctx) {
    HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, 
                                     __uuidof(ctx->dwrite_factory),
                                     (IUnknown**)&ctx->dwrite_factory);
    if (!SUCCEEDED(hr)) {
        log_err_return("DWriteFactory creation failed")
    }
}

void d2dl_loadFont(D2DLContext *ctx, const wchar_t *font_name, float size) {
    // TODO: Implement loading different font styles
    ctx->dwrite_factory->CreateTextFormat(
        font_name, NULL, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"",
        &ctx->text_format);
}

static void d2dl_createRenderTarget(D2DLContext *ctx) {
    RECT rc;
    GetClientRect(ctx->window, &rc);

    ID2D1HwndRenderTarget *rt = NULL;

    HRESULT hr = ctx->factory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(
          ctx->window, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)), &rt);

    if (SUCCEEDED(hr)) {
        ctx->rt = rt;
    }
    else {
        log_err_return("CreateHwndRenderTarget failed")
    }
}

D2DLContext *d2dl_initDesktopWindow(int width, int height, const char *window_title) {
    D2DLContext *ctx = (D2DLContext*)malloc(sizeof(D2DLContext));
    if (ctx == NULL) {
        log_err_return_null("D2DLContext allocation failed")
    }
    HWND hwnd = CreateWindow(window_title, window_title, WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, width,
                         height, NULL, NULL, NULL, ctx);
    if (hwnd == NULL) {
        log_err_return_null("CreateWindow failed");
    }
    ctx->window = hwnd;
    d2dl_createFactory(ctx);
    d2dl_createRenderTarget(ctx);
    d2dl_createDirectWriteFactory(ctx);

    ShowWindow(hwnd, SW_SHOW);
    return ctx;
}

void d2dl_deinit(D2DLContext *c) {
    DestroyWindow(c->window);
    c->rt->Release();
    c->factory->Release();
    c->dwrite_factory->Release();
    if (c->text_format)
        c->text_format->Release();
    free(c);
}

void d2dl_setParent(D2DLContext *c, HWND parent) {
    SetParent(c->window, parent);
}

void d2dl_setVisible(D2DLContext *c, bool visible) {
    ShowWindow(c->window, visible ? SW_SHOW : SW_HIDE);
}


#if 0

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE prevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
    const char class_name[] = "WIN32_WINDOW";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    
    D2DLContext *ctx = (D2DLContext*)malloc(sizeof(D2DLContext));

    HWND hwnd = CreateWindow(class_name, class_name, WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                         CW_USEDEFAULT, NULL, NULL, hInstance, ctx);
    if (hwnd == NULL) {
        fprintf(stderr, "CreateWindow failed\n");
        return 0;
    }
    ctx->window = hwnd;
    d2dl_createFactory(ctx);
    d2dl_createRenderTarget(ctx);
    d2dl_createDirectWriteFactory(ctx);
    d2dl_loadFont(ctx, L"Verdana", 32.f);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    d2dl_deinit(ctx);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        D2DLContext *ctx = (D2DLContext *)pcs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ctx);
        return 1;
    }
    D2DLContext *ctx = (D2DLContext*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_DISPLAYCHANGE:
        InvalidateRect(hwnd, NULL, false);
        return 0;
    case WM_SIZE:
        ctx->rt->Resize({(UINT)LOWORD(lParam), (UINT)HIWORD(lParam)});
        return 0;
    case WM_PAINT:
        RECT rc;
        GetClientRect(hwnd, &rc);
        d2dl_paint(ctx, rc);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif
