/**************************************************************************/
/*  remote_debugger_peer_web.cpp                                         */
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

#include "remote_debugger_peer_web.h"

#include "../godot_js.h"

#include "core/config/project_settings.h"
#include "core/io/marshalls.h"

bool RemoteDebuggerPeerWeb::is_peer_connected() {
	return connected;
}

bool RemoteDebuggerPeerWeb::has_message() {
	return in_queue.size() > 0;
}

Array RemoteDebuggerPeerWeb::get_message() {
	ERR_FAIL_COND_V(in_queue.is_empty(), Array());
	const Array msg = in_queue.front()->get();
	in_queue.pop_front();
	return msg;
}

Error RemoteDebuggerPeerWeb::put_message(const Array &p_arr) {
	if (out_queue.size() >= max_queued_messages) {
		return ERR_OUT_OF_MEMORY;
	}
	out_queue.push_back(p_arr);
	return OK;
}

int RemoteDebuggerPeerWeb::get_max_message_size() const {
	return 8 << 20; // 8 MiB, matches the other peer implementations.
}

void RemoteDebuggerPeerWeb::close() {
	godot_js_debug_ipc_close();
	connected = false;
	in_queue.clear();
	out_queue.clear();
}

void RemoteDebuggerPeerWeb::poll() {
	if (!connected) {
		return;
	}

	const CharString role_cs = role.utf8();
	while (in_queue.size() < max_queued_messages) {
		int size = godot_js_debug_ipc_avail(role_cs.get_data());
		if (size < 0) {
			break;
		}
		Vector<uint8_t> buf;
		buf.resize(size);
		int read = godot_js_debug_ipc_recv(role_cs.get_data(), buf.ptrw(), size);
		ERR_BREAK(read != size);
		Variant var;
		int r = 0;
		Error err = decode_variant(var, buf.ptr(), buf.size(), &r);
		ERR_CONTINUE(err != OK || var.get_type() != Variant::ARRAY);
		in_queue.push_back(var);
	}

	while (!out_queue.is_empty()) {
		const Array &msg = out_queue.front()->get();
		int size = 0;
		Error err = encode_variant(msg, nullptr, size);
		ERR_BREAK(err != OK);
		Vector<uint8_t> buf;
		buf.resize(size);
		encode_variant(msg, buf.ptrw(), size);
		godot_js_debug_ipc_send(role_cs.get_data(), buf.ptr(), size);
		out_queue.pop_front();
	}

	connected = godot_js_debug_ipc_is_open();
}

RemoteDebuggerPeerWeb::RemoteDebuggerPeerWeb(const String &p_role) :
		role(p_role) {
	max_queued_messages = (int)GLOBAL_GET("network/limits/debugger/max_queued_messages");
}

Ref<RemoteDebuggerPeer> RemoteDebuggerPeerWeb::create(const String &p_uri) {
	ERR_FAIL_COND_V(!p_uri.begins_with("webipc://"), nullptr);
	godot_js_debug_ipc_open("game");
	return memnew(RemoteDebuggerPeerWeb("game"));
}
