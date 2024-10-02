#include "cefclient.hpp"

#include <thread>
#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

void SimpleClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) 
{
    static bool needWindow = true;
    if (!needWindow) {
        return;
    }
    needWindow = false;
    browser_ = browser;

    const std::string fragmentShader =
        "uniform sampler2D texture;"
        "void main()"
        "{"
        "vec4 color = texture2D(texture, gl_TexCoord[0].xy);"
        "gl_FragColor = vec4(color.b, color.g, color.r, color.a);"
        "}";
    shaderBR.loadFromMemory(fragmentShader, sf::Shader::Fragment);
    shaderBR.setUniform("texture", sf::Shader::CurrentTexture);

    int dWidth = sf::VideoMode::getDesktopMode().width;
    size_ = sf::Vector2i(int(dWidth / 2.8f), int(dWidth / 2.5f));

    std::thread handleWindowThread(&SimpleClient::handleWindow, this);
    handleWindowThread.detach();
}

bool SimpleClient::DoClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed description of this
    // process.

    std::cout << "doClose\n";


    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return true;
}

void SimpleClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    std::cout << "onBeforeClose\n";

    CefQuitMessageLoop();
}

void SimpleClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    rect = CefRect(0, 0, size_.x, size_.y);
}

void SimpleClient::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, 
    const RectList& dirtyRects, const void* buffer, int width, int height)
{
    sf::Image img;
    img.create(width, height, (sf::Uint8*)buffer);

    sf::Texture texture;
    texture.loadFromImage(img);
    sf::Sprite sprite(texture);

    window_.clear();
    window_.draw(sprite, &shaderBR);
    window_.display();
    window_.setActive(false);
}

bool SimpleClient::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY)
{
    screenX = viewX + window_.getPosition().x;
    screenY = viewY + window_.getPosition().y;
    return true;
}

void SimpleClient::handleWindow()
{
    window_.create(sf::VideoMode(size_.x, size_.y), "Lyrics-viewer", sf::Style::Close);
    window_.clear(sf::Color(26, 26, 26));
    window_.display();
    window_.setActive(false);

    while (window_.isOpen()) {
        sf::Event e;
        if (!window_.waitEvent(e))
            continue;

        //window closed before login (error 201)
        if (e.type == sf::Event::Closed) {
            window_.close();
            std::cerr << "error 103\n";
            std::exit(201);
        }
        //send key stoke event
        else if (e.type == sf::Event::KeyPressed) {
            CefKeyEvent ev;
            ev.type = cef_key_event_type_t::KEYEVENT_KEYDOWN;

            ev.native_key_code = 0x09;
            ev.windows_key_code = 0x09;
            

            browser_->GetHost()->SendKeyEvent(ev);
        }
        //send mouse click event
        else if (e.type == sf::Event::MouseButtonPressed || e.type == sf::Event::MouseButtonReleased) {
            CefBrowserHost::MouseButtonType type;
            if (e.mouseButton.button == sf::Mouse::Left)
                type = CefBrowserHost::MouseButtonType::MBT_LEFT;
            else if (e.mouseButton.button == sf::Mouse::Middle)
                type = CefBrowserHost::MouseButtonType::MBT_MIDDLE;
            else
                type = CefBrowserHost::MouseButtonType::MBT_RIGHT;

            CefMouseEvent ev;
            ev.x = e.mouseButton.x;
            ev.y = e.mouseButton.y;
            
            if (e.type == sf::Event::MouseButtonPressed)
                browser_->GetHost()->SendMouseClickEvent(ev, type, false, 1);
            else
                browser_->GetHost()->SendMouseClickEvent(ev, type, true, 1);
        }
        //send mouse moved event
        else if (e.type == sf::Event::MouseMoved || e.type == sf::Event::MouseLeft) {
            CefMouseEvent ev;
            ev.x = e.mouseMove.x;
            ev.y = e.mouseMove.y;

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                ev.modifiers += cef_event_flags_t::EVENTFLAG_LEFT_MOUSE_BUTTON;
            if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
                ev.modifiers += cef_event_flags_t::EVENTFLAG_MIDDLE_MOUSE_BUTTON;
            if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                ev.modifiers += cef_event_flags_t::EVENTFLAG_RIGHT_MOUSE_BUTTON;

            if (e.type == sf::Event::MouseLeft)
                browser_->GetHost()->SendMouseMoveEvent(ev, true);
            else
                browser_->GetHost()->SendMouseMoveEvent(ev, false);
        }
        //send mouse wheel scrolled event
        else if (e.type == sf::Event::MouseWheelScrolled) {
            CefMouseEvent ev;
            ev.x = e.mouseWheelScroll.x;
            ev.y = e.mouseWheelScroll.y;

            int delta = int(e.mouseWheelScroll.delta * 50);
            browser_->GetHost()->SendMouseWheelEvent(ev, 0, delta);
        }

        //resizing is not supported
        /*else if (e.type == sf::Event::Resized) {
            size_ = sf::Vector2f(window_.getSize());
            window_.setView(sf::View(sf::Vector2f(size_.x / 2.f, size_.y / 2.f), size_));

            browser_->GetHost()->WasResized();
            std::cout << "resized\n";
        }*/
    }
}
