/*
 * GameChat.cpp
 *
 *  Created on: May 27, 2012
 *      Author: 100397561
 */

#include "GameChat.h"
#include "GameState.h"

#include "../display/display.h"

static void print_dupe_string(const ChatMessage& cm, const font_data& font,
		const Pos& location, float alpha) {
	if (cm.exact_copies > 1)
		gl_printf(font, Colour(0, 191, 255, alpha * 255), location.x,
				location.y, " x%d", cm.exact_copies);
}

void ChatMessage::draw(const font_data& font, float alpha, int x, int y) const {
	Colour sendcol = sender_colour, msgcol = message_colour;
	sendcol.a *= alpha, msgcol.a *= alpha;
	Pos offset(0, 0);
	if (!sender.empty()) {
		offset = gl_printf(font, sendcol, x, y, "%s: %s", sender.c_str());
		x += offset.x;
	}
	offset = gl_printf(font, msgcol, x, y, message.c_str());
	x += offset.x;
	print_dupe_string(*this, font, Pos(x, y), alpha);
}

bool ChatMessage::empty() const {
	return sender.empty() && message.empty();
}

void GameChat::add_message(const ChatMessage& cm) {
	bool dupe = false;

	if (!messages.empty()) {
		if (messages.back() == cm) {
			messages.back().exact_copies++;
			dupe = true;
		}
	}

	if (!dupe)
		messages.push_back(cm);

	fade_out = 1.0f;
	fade_out_rate = 0.0025f;
}

void GameChat::add_message(const std::string& msg, const Colour& colour) {
	add_message(ChatMessage("", msg, Colour(), colour));
}

bool GameChat::is_typing_message() {
	return typing_message;
}

void GameChat::draw_player_chat(GameState* gs) const {
	const font_data& font = gs->primary_font();
	const int padding = 5;
	int line_sep = font.h + 1;

	int view_w = gs->window_view().width, view_h = gs->window_view().height;
	int chat_w = view_w, chat_h = 100;
	int chat_x = 0, chat_y = 0; //h - chat_h - TILE_SIZE;
	int text_x = chat_x + padding, text_y = chat_y + padding;

	gl_set_drawing_area(0, 0, view_w, view_h);

	gl_draw_rectangle(chat_x, chat_y, chat_w, chat_h,
			Colour(180, 180, 255, 50 * fade_out));

	bool draw_typed_message = typing_message || !current_message.empty();

	int start_msg = 0;
	int message_space = chat_h - padding * 2
			- (draw_typed_message ? line_sep : 0);
	int msgs_in_screen = message_space / line_sep;
	if (messages.size() > msgs_in_screen) {
		start_msg = messages.size() - msgs_in_screen;
	}

	for (int i = start_msg; i < messages.size(); i++) {
		messages[i].draw(font, fade_out, text_x, text_y);
		text_y += line_sep;
	}

	if (draw_typed_message) {
		int type_y = chat_y + chat_h - padding - line_sep;
		current_message.draw(font, fade_out, text_x, type_y);
	}
}

void GameChat::step(GameState *gs) {
	if (gs->key_press_state(SDLK_c)) {
		show_chat = !show_chat;
		fade_out_rate = 0.05f;
	}
	if (show_chat)
		fade_out = 1.0f;
	else if (fade_out > 0.0f)
		fade_out -= fade_out_rate;
}
void GameChat::draw(GameState *gs) const {
	if (fade_out > 0.0f)
		draw_player_chat(gs);
}

GameChat::GameChat() :
		current_message("", "This is My Message") {
	show_chat = false;
	fade_out = 0.0f;
	fade_out_rate = 0.05f;
	typing_message = false;
//	messages.push_back(
//			ChatMessage("ludamad", "What's up!?\nGo eff off",
//					Colour(37, 207, 240)));
//	messages.push_back(ChatMessage("ciribot", "nm u", Colour(255, 69, 0)));

//	char buff[40];
//	for (int i = 0; i < 14; i++){
//		snprintf(buff, 40, "Message %d\n", i);
//		this->add_message(buff);
//	}
}
