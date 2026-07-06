/**************************************************************************/
/*  editor_debugger_server_web.cpp                                       */
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

#include "editor_debugger_server_web.h"

#include "../debugger/remote_debugger_peer_web.h"
#include "../godot_js.h"

Ref<EditorDebuggerServer> EditorDebuggerServerWeb::create(const String &p_protocol) {
	ERR_FAIL_COND_V(p_protocol != "webipc://", nullptr);
	return memnew(EditorDebuggerServerWeb);
}

String EditorDebuggerServerWeb::get_uri() const {
	return "webipc://local";
}

Error EditorDebuggerServerWeb::start(const String &p_uri) {
	godot_js_debug_ipc_open("editor");
	active = true;
	return OK;
}

void EditorDebuggerServerWeb::stop() {
	godot_js_debug_ipc_close();
	active = false;
}

bool EditorDebuggerServerWeb::is_active() const {
	return active;
}

bool EditorDebuggerServerWeb::is_connection_available() const {
	return active && godot_js_debug_ipc_is_connection_available();
}

Ref<RemoteDebuggerPeer> EditorDebuggerServerWeb::take_connection() {
	ERR_FAIL_COND_V(!is_connection_available(), Ref<RemoteDebuggerPeer>());
	godot_js_debug_ipc_accept();
	return memnew(RemoteDebuggerPeerWeb("editor"));
}
