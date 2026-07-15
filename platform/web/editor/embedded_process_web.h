/**************************************************************************/
/*  embedded_process_web.h                                                */
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

#pragma once

#include "editor/run/embedded_process.h"

// Web has no true window embedding (the game runs as a second WASM instance in the
// same page, see `editor.html`). This pseudo-embedder positions a DOM overlay over
// the Game View's control area instead of calling `DisplayServer::embed_process`.
class EmbeddedProcessWeb : public EmbeddedProcessBase {
	GDCLASS(EmbeddedProcessWeb, EmbeddedProcessBase);

	ProcessID current_process_id = 0;
	bool embedding_completed = false;
	bool overlay_visible = false;

	void _update_embedded_process();

protected:
	void _notification(int p_what);

public:
	bool is_embedding_in_progress() const override { return false; }
	bool is_embedding_completed() const override { return embedding_completed; }
	bool is_process_focused() const override { return embedding_completed && has_focus(); }
	void embed_process(ProcessID p_pid) override;
	int get_embedded_pid() const override { return current_process_id; }
	void reset() override;
	void reset_timers() override {}
	void request_close() override;
	void queue_update_embedded_process() override;

	Rect2i get_adjusted_embedded_window_rect(const Rect2i &p_rect) const override;

	EmbeddedProcessWeb();
	~EmbeddedProcessWeb() override;
};
