#include <glad/glad.h>
#include "Gui.hpp"
#include <Utils/InputManager.hpp>
#include "Common/Gui_Types.hpp"
#include "Utils/Interpolater.hpp"

using namespace gui;
using namespace ease;

void Gui::Multiplayer(const bool Initialize) {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};
    static Animation_State<glm::vec2> Back_State;
    static Animation_State<glm::vec2> Wip_State;
    static Animation_State<glm::vec2> Host_State;
    static Animation_State<glm::vec2> Join_State;
    if (Initialize) {
        return;
    }
    Layout layout = {Anch::Center, {150, 20}, {0, 50}};
    Label label = {.anchor = Anch::Center};

    layout.Offset = {0, 50};
    label.text = "Back";
    if (Button(layout, Big, label, &Back_State)) {
        Back_State.state = State::Idle;
        game.MenuId = 0;
    }

    layout.Offset.y = 25;
    label.text = "Last (wip)";
    if (Button(layout, Big, label, &Wip_State)) {
        Wip_State.state = State::Idle;
    }

    layout.Offset.y = -25;
    label.text = "Host";
    if (Button(layout, Big, label, &Host_State)) {
        Host_State.state = State::Idle;
        game.MenuId = 4;
    }

    layout.Offset.y = 0;
    layout.Size.x = 70;
    label.text = "Join";
    if (Button(layout, Small, label, &Join_State)) {
        Join_State.state = State::Idle;
        game.MenuId = 3;
    }
}

void Gui::MultiplayerJoin() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};

    Layout layout = {Anch::Center, {70, 20}, {0, 25}};
    Label label = {.anchor = Anch::Center};
    static Animation_State<glm::vec2> Join_State;
    static Animation_State<glm::vec2> Back_State;
    static Animation_State<glm::vec2> Port_State;
    static Animation_State<glm::vec2> Ip_State;

    label.text = (!game.Joined) ? "Join" : "Disconect";
    if (Button(layout, Small, label, &Join_State)) {
        Join_State.state = State::Idle;
        if (game.Joined) {
            net.client.Request_Stop();
            game.Joined = false;
        } else {
            net.Start_Client("127.0.0.1", 25565, "PLSiorbpl");
            game.Joined = true;
        }
    }

    layout.Size = {150, 20};
    layout.Offset.y = 50;
    label.text = "Back";
    if (Button(layout, Big, label, &Back_State)) {
        Back_State.state = State::Idle;
        game.MenuId = 2;
    }

    layout.Offset.y = 0;
    label.anchor = Anch::LeftCenter;
    label.Offset.x = 3;
    label.text = "Port:";
    if (Button(layout, Big, label, &Port_State)) {
        Port_State.state = State::Idle;
    }

    layout.Offset.y = -25;
    label.text = "Ip:";
    if (Button(layout, Big, label, &Ip_State)) {
        Ip_State.state = State::Idle;
    }
}

void Gui::MultiplayerHost() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};
    static TextInputStyle Host_style = {.style = Big, .Default_str = "Enter port:", .max_chars = 5, .Input_Mode = 1};
    static std::string HostInput;
    static Animation_State<glm::vec2> Host_State;
    static Animation_State<glm::vec2> Back_State;
    static Animation_State<glm::vec2> Port_State;

    Layout layout = {Anch::Center, {70, 20}, {0, 0}};
    Label label = {.anchor = Anch::Center};

    label.text = (!game.Hosting) ? "Host" : "Stop";
    if (Button(layout, Small, label, &Host_State)) {
        Host_State.state = State::Idle;
        if (game.Hosting) {
            net.server.Stop_Server();
            game.Hosting = false;
        } else {
            try {
                if (std::stoi(HostInput) > 0 && std::stoi(HostInput.c_str()) < 65535) {
                    net.Start_Server(25565);
                    game.Hosting = true;
                }
            } catch (...) {
                chat.push_back("Wrong Input");
            }
        }
    }

    layout.Size.x = 150;
    layout.Offset.y = 25;
    label.text = "Back";
    if (Button(layout, Big, label, &Back_State)) {
        Back_State.state = State::Idle;
        game.MenuId = 2;
        InputManager::InputActive = false;
    }

    layout.Offset.y = -25;
    label.anchor = Anch::LeftCenter;
    label.Offset.x = 3;
    label.text = HostInput;
    if (TextInput(layout, Host_style, label, &Port_State)) {
        Port_State.state = State::Idle;
    }
    HostInput = label.text;
}
