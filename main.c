#include "d2dl.h"

#include <stdio.h>
#include <math.h>

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
    
    D2DLContext *ctx = d2dl_initDesktopWindow(900, 600, class_name);
    d2dl_loadFont(ctx, L"Verdana", 32.f);

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
    case WM_SIZE:
        d2dl_setRenderSize(ctx, (D2DSize){(float)LOWORD(lParam), (float)HIWORD(lParam)});
        return 0;
    case WM_PAINT:
        D2DSize size = d2dl_getRenderSize(ctx);
        float cx = size.width / 2;
        float cy = size.height / 2;
        d2dl_beginDrawing(ctx);
        d2dl_clear(ctx, (D2DColor){.r = 0.5f, .g = 0.5f, .b = 0.5f, .a = 1.f});
        d2dl_drawRect(ctx, (D2DRect){.left = 5, .top = 5,
                      .right = size.width - 5, .bottom = size.height - 5},
                      5.f, Green);
        const float r = fminf(size.width/2, size.height/2);
        d2dl_fillEllipse(ctx, (D2DPoint){cx, cy}, r, r, Blue);
        d2dl_drawText(ctx, L"WOH!!! It's wourkin!",
                      (D2DRect){
                          .left = cx - r, .top = cy - r,
                          .right = cx + r, .bottom = cy + r},
                        Black);
        d2dl_endDrawing(ctx);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
