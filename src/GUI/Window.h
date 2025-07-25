#pragma once
#include <filesystem>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdkmm/display.h>
#include <glibmm/priorities.h>
#include <gtkmm/enums.h>
#include <gtkmm/eventcontroller.h>
#include <gtkmm/object.h>
#include <gtkmm/separator.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/entry.h>
#include "../HttpManager/HttpManager.h"
#include <gtkmm/label.h>
#include <openssl/pkcs7.h>
#include "../Interpreter/Lexer.h"
#include "../Interpreter/Parser.h"
#include "../Interpreter/Interpreter.h"
#include <gtkmm/scrolledwindow.h>
#include <sigc++/functors/mem_fun.h>

class BrowserWindow : public Gtk::Window {
    HttpManager& http_manager;
    Gtk::Label* content_test_label = Gtk::manage(new Gtk::Label("hello"));
    Gtk::Entry* url_input;
    Lexer lexer;
    Parser parser;
    Interpreter interpreter;

    Gtk::ScrolledWindow* content_box_member;
    Gtk::Box* actual_box_member;

public:

    bool on_close_request() override {
        std::vector<std::shared_ptr<HTMLTag>> tagg;
        interpreter.current_tags[0]->flatten(tagg);
        for(auto& thing : tagg){
            thing->parent_widget = nullptr; // cancell deletion cause handled auto
        }
        EVP_cleanup();
        ERR_free_strings();
        return false;
    }
    BrowserWindow(HttpManager& httpManager): http_manager(httpManager){
        set_default_size(600, 500);

        auto child_box = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
        
        content_box_member = Gtk::manage(new Gtk::ScrolledWindow);
        content_box_member->set_hexpand(true);
        content_box_member->set_halign(Gtk::Align::FILL);
        content_box_member->set_vexpand(true);
        content_box_member->set_valign(Gtk::Align::FILL);
        content_box_member->add_css_class("content_box");

        actual_box_member = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
        actual_box_member->set_hexpand(true);
        actual_box_member->set_halign(Gtk::Align::FILL);
        actual_box_member->set_vexpand(true);
        actual_box_member->set_valign(Gtk::Align::FILL);

        content_box_member->set_child(*actual_box_member);

        content_test_label->add_css_class("content_label");
        content_test_label->set_hexpand(true);
        content_test_label->set_valign(Gtk::Align::START);
        content_test_label->set_halign(Gtk::Align::START);
        content_test_label->set_wrap();
        content_test_label->set_max_width_chars(30);


        auto url_box = Gtk::manage(new Gtk::Box(Gtk::Orientation::HORIZONTAL));
        url_box->set_hexpand(true);
        url_box->set_halign(Gtk::Align::FILL);
        url_box->set_valign(Gtk::Align::START);
        url_box->add_css_class("url_box");

        auto controller = Gtk::EventControllerKey::create();
        controller->signal_key_released().connect([=, this](guint keyval, guint keycode, Gdk::ModifierType state){
            if(keyval == GDK_KEY_Return && url_input->get_text_length() > 0){

                std::string html_content = http_manager.getRequest(url_input->get_text());
                auto tokens = lexer.tokenize(html_content);
                auto program = parser.produceAst(tokens);
                interpreter.renderTags(actual_box_member, program);
            }
        });
        
        auto first_spacer_box = Gtk::manage(new Gtk::Box);
        first_spacer_box->set_halign(Gtk::Align::START);
        first_spacer_box->add_css_class("url_box_first_spacer");

        url_input = Gtk::manage(new Gtk::Entry);
        url_input->set_hexpand(true);
        url_input->set_halign(Gtk::Align::FILL);
        url_input->set_valign(Gtk::Align::CENTER);
        url_input->add_css_class("url_input");
        url_input->set_placeholder_text("Enter A Url");
        url_input->add_controller(controller);

        auto last_spacer_box = Gtk::manage(new Gtk::Box);
        last_spacer_box->set_halign(Gtk::Align::START);
        last_spacer_box->add_css_class("url_box_last_spacer");

        url_box->append(*first_spacer_box);
        url_box->append(*url_input);
        url_box->append(*last_spacer_box);

        child_box->append(*url_box);
        child_box->append(*content_box_member);

        set_child(*child_box);

        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_path("main.css");

        Gtk::CssProvider::add_provider_for_display(
            Gdk::Display::get_default(), css_provider,
             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    }
};
