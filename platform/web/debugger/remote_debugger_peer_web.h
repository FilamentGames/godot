/**************************************************************************/
/*  remote_debugger_peer_web.h                                           */
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

#include "core/debugger/remote_debugger_peer.h"

// Same-page debugger transport for the web editor. The editor and the game
// it runs both live in the same page as separate Emscripten instances (see
// `editor.html`), so messages are exchanged through a small JS-side queue
// (`library_godot_debug_ipc.js`) instead of a real socket. Used for both
// ends of the connection (role "game" here, role "editor" on the editor
// side), since the wire format and queueing logic are identical.
class RemoteDebuggerPeerWeb : public RemoteDebuggerPeer {
	GDSOFTCLASS(RemoteDebuggerPeerWeb, RemoteDebuggerPeer);

	String role;
	List<Array> in_queue;
	List<Array> out_queue;
	bool connected = true;

public:
	static Ref<RemoteDebuggerPeer> create(const String &p_uri);

	bool is_peer_connected() override;
	int get_max_message_size() const override;
	bool has_message() override;
	Error put_message(const Array &p_arr) override;
	Array get_message() override;
	void close() override;
	void poll() override;
	bool can_block() const override { return false; }

	RemoteDebuggerPeerWeb(const String &p_role);
};
