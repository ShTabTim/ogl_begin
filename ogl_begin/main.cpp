#include <windows.h>
#include <gl/gl.h>
#pragma comment(lib, "opengl32.lib")

#include <chrono>
#include <thread>
#include <string>
#include <atomic>

std::atomic<HWND> hwnd;
std::atomic<double> pt = 0;
std::atomic<bool> is_active = true;
std::atomic<bool> is_not_pause = true;
std::atomic<bool> is_resizing = false;
std::atomic<uint32_t> width;
std::atomic<uint32_t> height;

void _draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();
    glRotatef(pt * 180 / 3.14159265, 0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLES);

    glColor3f(1.0f, 0.0f, 0.0f);   glVertex2f(0.0f, 1.0f);
    glColor3f(0.0f, 1.0f, 0.0f);   glVertex2f(0.87f, -0.5f);
    glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(-0.87f, -0.5f);

    glEnd();

    glPopMatrix();
}

void _update(double dt) {
    pt = pt + dt;
}

void __process() {
    double pdt = 0.0001f;
    std::chrono::high_resolution_clock::time_point old_time = std::chrono::high_resolution_clock::now();
    Sleep(1);
    std::chrono::high_resolution_clock::time_point new_time = std::chrono::high_resolution_clock::now();
    Sleep(1);
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - old_time);
    while (is_active) {
        new_time = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - old_time);
        pdt = time_span.count();
        old_time = new_time;

        if (is_not_pause) {
            _update(pdt);
        }
    }
}
void __draw_process() {
    HDC hDC;
    HGLRC hRC;
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(hDC, &pfd);

    SetPixelFormat(hDC, iFormat, &pfd);

    hRC = wglCreateContext(hDC);

    double fdt = 0.00001f;
    std::chrono::high_resolution_clock::time_point old_time = std::chrono::high_resolution_clock::now();
    Sleep(1);
    std::chrono::high_resolution_clock::time_point new_time = std::chrono::high_resolution_clock::now();
    Sleep(1);
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - old_time);


    wglMakeCurrent(hDC, hRC);
    while (is_active) {
        if (is_resizing) {
            is_resizing = false;
            glViewport(0, 0, width, height);
        }
        new_time = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - old_time);
        fdt = time_span.count();
        old_time = new_time;

        if (is_not_pause) {
            _draw();
            SwapBuffers(hDC);
        }
    }
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_SIZE:
        width = LOWORD(lparam);
        height = HIWORD(lparam);
        is_resizing = true;
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_DESTROY:
        return 0;
    case WM_KEYDOWN: {
        switch (wparam) {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        case VK_SPACE:
            is_not_pause = !is_not_pause;
            break;
        }
    } break;
    default:
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return 0;
}

int32_t WINAPI WinMain(HINSTANCE hinst, HINSTANCE prevhinst, LPSTR cmdline, int cmdshow) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hinst;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassExW(&wcex))
        return 0;

    hwnd = CreateWindowExW(0,
        wcex.lpszClassName,
        L"OpenGL Sample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        256,
        256,
        NULL,
        NULL,
        hinst,
        NULL);

    ShowWindow(hwnd, cmdshow);

    std::thread process = std::thread(&__process);
    std::thread draw_process = std::thread(&__draw_process);

    MSG msg;
    do {
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (msg.message != WM_QUIT);

    is_active = false;
    process.join();
    draw_process.join();

    DestroyWindow(hwnd);

    return msg.wParam;
}