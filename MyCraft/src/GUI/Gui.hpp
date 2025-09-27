#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

class Widget {
public:
    float x, y;
    float w, h;
    bool visible = true;

    virtual void update(double mouseX, double mouseY, bool mousePressed) = 0;
    virtual void render() = 0;
    virtual ~Widget() {}
};

class Label : public Widget {
public:
    std::string text;

    Label(float x, float y, std::string text) {
        this->x = x; this->y = y;
        this->w = 0; this->h = 0;
        this->text = text;
    }

    void update(double, double, bool) override {}
    void render() override {
        std::cout << "Render Label: " << text << "\n";
    }
};

class Button : public Widget {
public:
    std::string text;
    bool hovered = false;
    bool pressed = false;

    Button(float x, float y, float w, float h, std::string txt) {
        this->x = x; this->y = y; this->w = w; this->h = h;
        this->text = txt;
    }

    void update(double mouseX, double mouseY, bool mousePressed) override {
        hovered = (mouseX >= x && mouseX <= x+w && mouseY >= y && mouseY <= y+h);
        pressed = hovered && mousePressed;
    }

    void render() override {
        std::cout << "Render Button: " << text
                  << (pressed ? " [PRESSED]" : hovered ? " [HOVER]" : "") << "\n";
    }
};

class Gui {
public:
    std::vector<std::unique_ptr<Widget>> widgets;

    template <typename T, typename... Args>
    T* addWidget(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = widget.get();
        widgets.push_back(std::move(widget));
        return ptr;
    }

    void update(double mouseX, double mouseY, bool mousePressed) {
        for (auto& w : widgets) if (w->visible) w->update(mouseX, mouseY, mousePressed);
    }

    void render() {
        for (auto& w : widgets) if (w->visible) w->render();
    }
};
