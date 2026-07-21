#include <glad/glad.h>
#include "Gui.hpp"
#include <Utils/InputManager.hpp>
#include "Common/Gui_Types.hpp"
#include "Utils/Interpolater.hpp"

using namespace gui;
using namespace ease;

void Gui::Multiplayer() {
    ID = 0;
    static constexpr ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static constexpr ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};

    // Back Button
    static Button_Widget Back = {{Anch::Center, {150, 20}, {0, 50}},
    Big,
    {"Back", .anchor = Anch::Center}
    };
    if (Button(Back.layout, Back.style, Back.label, &Back.state)) {
        Back.state.state = State::Idle;
        game.MenuId = 0;
    }

    // Last (Wip) Button
    static Button_Widget Last = {{Anch::Center, {150, 20}, {0, 25}},
    Big,
    {"Last (wip)", .anchor = Anch::Center}
    };
    if (Button(Last.layout, Last.style, Last.label, &Last.state)) {
        Last.state.state = State::Idle;
    }

    // Host Button
    static Button_Widget Host = {{Anch::Center, {150, 20}, {0, -25}},
    Big,
    {"Host", .anchor = Anch::Center}
    };
    if (Button(Host.layout, Host.style, Host.label, &Host.state)) {
        Host.state.state = State::Idle;
        game.MenuId = 4;
    }

    // Join Button
    static Button_Widget Join = {{Anch::Center, {70, 20}, {0, 0}},
    Small,
    {"Join", .anchor = Anch::Center}
    };
    if (Button(Join.layout, Join.style, Join.label, &Join.state)) {
        Join.state.state = State::Idle;
        game.MenuId = 3;
    }
}

void Gui::MultiplayerJoin() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};
    static TextInputStyle Port_style = {.style = Big, .Default_str = "Enter port:", .max_chars = 5, .Input_Mode = 1};
    static std::string PortInput;
    static TextInputStyle Ip_style = {.style = Big, .Default_str = "Enter Ipv4:", .max_chars = 15, .Input_Mode = 0};
    static std::string IpInput;

    static Animation_State<glm::vec2> Join_State;
    static Animation_State<glm::vec2> Back_State;
    static Animation_State<glm::vec2> Port_State;
    static Animation_State<glm::vec2> Ip_State;

    Layout layout = {Anch::Center, {70, 20}, {0, 25}};
    Label label = {.anchor = Anch::Center};

    label.text = (!game.Joined) ? "Join" : "Disconect";
    if (Button(layout, Small, label, &Join_State)) {
        Join_State.state = State::Idle;
        if (game.Joined) {
            net.client.Request_Stop();
            game.Joined = false;
        } else {
            try {
                if (std::stoi(PortInput) > 0 && std::stoi(PortInput) < 65535) {
                    net.Start_Client(IpInput, std::stoi(PortInput), "PLSiorbpl");
                    game.Joined = true;
                }
            } catch (...) {
                chat.push_back("Unable to connect: Wrong Input");
            }
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
    label.text = PortInput;
    if (TextInput(layout, Port_style, label, &Port_State)) {
        Port_State.state = State::Idle;
    }
    PortInput = label.text;

    layout.Offset.y = -25;
    label.text = IpInput;
    if (TextInput(layout, Ip_style, label, &Ip_State)) {
        Ip_State.state = State::Idle;
    }
    IpInput = label.text;
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
                if (std::stoi(HostInput) > 0 && std::stoi(HostInput) < 65535) {
                    net.Start_Server(25565);
                    game.Hosting = true;
                }
            } catch (...) {
                chat.push_back("Unable to start server: Wrong Input");
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
