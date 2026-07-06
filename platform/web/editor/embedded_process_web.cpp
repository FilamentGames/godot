/**************************************************************************/
/*  embedded_process_web.cpp                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "embedded_process_web.h"

#include "platform/web/godot_js.h"

Rect2i EmbeddedProcessWeb::get_adjusted_embedded_window_rect(const Rect2i &p_rect) const {
	Rect2i control_rect = Rect2i(p_rect.position + margin_top_left, (p_rect.size - get_margins_size()).maxi(1));
	if (window_size != Size2i()) {
		Rect2i desired_rect;
		if (!keep_aspect && control_rect.size.x >= window_size.x && control_rect.size.y >= window_size.y) {
			// Fixed at the desired size.
			desired_rect.size = window_size;
		} else {
			float ratio = MIN((float)control_rect.size.x / window_size.x, (float)control_rect.size.y / window_size.y);
			desired_rect.size = Size2i(window_size.x * ratio, window_size.y * ratio).maxi(1);
		}
		desired_rect.position = Size2i(control_rect.position.x + ((control_rect.size.x - desired_rect.size.x) / 2), control_rect.position.y + ((control_rect.size.y - desired_rect.size.y) / 2));
		return desired_rect;
	} else {
		// Stretch, use all the control area.
		return control_rect;
	}
}

void EmbeddedProcessWeb::embed_process(ProcessID p_pid) {
	reset();

	current_process_id = p_pid;
	set_notify_transform(true);

	// The game is already launching asynchronously via `onExecute` in editor.html;
	// there's no handshake to wait for, so embedding is considered complete immediately.
	embedding_completed = true;
	_update_embedded_process();
	queue_redraw();
	emit_signal(SNAME("embedding_completed"));
}

void EmbeddedProcessWeb::reset() {
	if (overlay_visible) {
		godot_js_game_embed_hide();
		overlay_visible = false;
	}
	current_process_id = 0;
	embedding_completed = false;
	set_notify_transform(false);
	queue_redraw();
}

void EmbeddedProcessWeb::request_close() {
	if (current_process_id != 0 && embedding_completed) {
		godot_js_os_request_game_quit();
	}
}

void EmbeddedProcessWeb::queue_update_embedded_process() {
	_update_embedded_process();
}

void EmbeddedProcessWeb::_update_embedded_process() {
	if (!embedding_completed) {
		return;
	}
	if (!is_visible_in_tree()) {
		if (overlay_visible) {
			godot_js_game_embed_hide();
			overlay_visible = false;
		}
		return;
	}

	Rect2i r = get_screen_embedded_window_rect();
	if (!overlay_visible) {
		godot_js_game_embed_show(r.position.x, r.position.y, r.size.x, r.size.y);
		overlay_visible = true;
	} else {
		godot_js_game_embed_update(r.position.x, r.position.y, r.size.x, r.size.y);
	}
}

void EmbeddedProcessWeb::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_RESIZED:
		case NOTIFICATION_VISIBILITY_CHANGED:
		case NOTIFICATION_TRANSFORM_CHANGED: {
			queue_update_embedded_process();
		} break;
	}
}

EmbeddedProcessWeb::EmbeddedProcessWeb() {
}

EmbeddedProcessWeb::~EmbeddedProcessWeb() {
	if (current_process_id != 0) {
		reset();
	}
}
