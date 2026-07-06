/**************************************************************************/
/*  library_godot_debug_ipc.js                                           */
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

// Same-page debugger transport (`webipc://`). The editor and the game it
// launches both run as separate Emscripten instances in the same page (see
// `editor.html`), so instead of `postMessage` (async, structured-clone
// overhead, and both instances would observe every message) we use a plain
// FIFO queue pair shared via `globalThis`. This keeps the channel isolated
// here so it could be swapped for `MessageChannel`/`postMessage` later if the
// game ever moves into an iframe or worker.
const GodotDebugIPC = {
	$GodotDebugIPC__deps: ['$GodotRuntime'],
	$GodotDebugIPC: {
		channel: function () {
			if (!globalThis['GodotDebugIPC']) {
				globalThis['GodotDebugIPC'] = {
					connected: false,
					accepted: false,
					toEditor: [],
					toGame: [],
				};
			}
			return globalThis['GodotDebugIPC'];
		},
	},

	godot_js_debug_ipc_open__proxy: 'sync',
	godot_js_debug_ipc_open__sig: 'vi',
	godot_js_debug_ipc_open: function (p_role) {
		const channel = GodotDebugIPC.channel();
		if (GodotRuntime.parseString(p_role) === 'game') {
			// A new game session; drop anything left over from a previous one.
			channel.connected = true;
			channel.accepted = false;
			channel.toEditor.length = 0;
			channel.toGame.length = 0;
		}
	},

	godot_js_debug_ipc_close__proxy: 'sync',
	godot_js_debug_ipc_close__sig: 'v',
	godot_js_debug_ipc_close: function () {
		const channel = GodotDebugIPC.channel();
		channel.connected = false;
		channel.accepted = false;
		channel.toEditor.length = 0;
		channel.toGame.length = 0;
	},

	godot_js_debug_ipc_is_open__proxy: 'sync',
	godot_js_debug_ipc_is_open__sig: 'i',
	godot_js_debug_ipc_is_open: function () {
		return GodotDebugIPC.channel().connected ? 1 : 0;
	},

	godot_js_debug_ipc_is_connection_available__proxy: 'sync',
	godot_js_debug_ipc_is_connection_available__sig: 'i',
	godot_js_debug_ipc_is_connection_available: function () {
		const channel = GodotDebugIPC.channel();
		return (channel.connected && !channel.accepted) ? 1 : 0;
	},

	godot_js_debug_ipc_accept__proxy: 'sync',
	godot_js_debug_ipc_accept__sig: 'v',
	godot_js_debug_ipc_accept: function () {
		GodotDebugIPC.channel().accepted = true;
	},

	godot_js_debug_ipc_send__proxy: 'sync',
	godot_js_debug_ipc_send__sig: 'viii',
	godot_js_debug_ipc_send: function (p_role, p_ptr, p_len) {
		const channel = GodotDebugIPC.channel();
		const queue = GodotRuntime.parseString(p_role) === 'editor' ? channel.toGame : channel.toEditor;
		queue.push(GodotRuntime.heapSlice(HEAPU8, p_ptr, p_len));
	},

	godot_js_debug_ipc_avail__proxy: 'sync',
	godot_js_debug_ipc_avail__sig: 'ii',
	godot_js_debug_ipc_avail: function (p_role) {
		const channel = GodotDebugIPC.channel();
		const queue = GodotRuntime.parseString(p_role) === 'editor' ? channel.toEditor : channel.toGame;
		return queue.length > 0 ? queue[0].length : -1;
	},

	godot_js_debug_ipc_recv__proxy: 'sync',
	godot_js_debug_ipc_recv__sig: 'iiii',
	godot_js_debug_ipc_recv: function (p_role, p_ptr, p_len) {
		const channel = GodotDebugIPC.channel();
		const queue = GodotRuntime.parseString(p_role) === 'editor' ? channel.toEditor : channel.toGame;
		if (queue.length === 0) {
			return 0;
		}
		const msg = queue[0];
		if (msg.length > p_len) {
			return -1; // Buffer too small, caller should retry with more room.
		}
		queue.shift();
		GodotRuntime.heapCopy(HEAPU8, msg, p_ptr);
		return msg.length;
	},
};
autoAddDeps(GodotDebugIPC, '$GodotDebugIPC');
mergeInto(LibraryManager.library, GodotDebugIPC);
