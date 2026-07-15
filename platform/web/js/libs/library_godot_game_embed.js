/**************************************************************************/
/*  library_godot_game_embed.js                                          */
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

// Web editor "embedding": the game runs as a second WASM instance in the same
// page (see `editor.html`), so there's no real window to embed. Instead, the
// editor tells the page to show/position/hide a DOM overlay above the Game
// View panel via `globalThis.godotGameEmbed`, defined in `editor.html`.
const GodotGameEmbed = {
	godot_js_game_embed_show__proxy: 'sync',
	godot_js_game_embed_show__sig: 'viiii',
	godot_js_game_embed_show: function (p_x, p_y, p_w, p_h) {
		const embed = globalThis['godotGameEmbed'];
		if (embed) {
			embed['show'](p_x, p_y, p_w, p_h);
		}
	},

	godot_js_game_embed_update__proxy: 'sync',
	godot_js_game_embed_update__sig: 'viiii',
	godot_js_game_embed_update: function (p_x, p_y, p_w, p_h) {
		const embed = globalThis['godotGameEmbed'];
		if (embed) {
			embed['update'](p_x, p_y, p_w, p_h);
		}
	},

	godot_js_game_embed_hide__proxy: 'sync',
	godot_js_game_embed_hide__sig: 'v',
	godot_js_game_embed_hide: function () {
		const embed = globalThis['godotGameEmbed'];
		if (embed) {
			embed['hide']();
		}
	},
};
mergeInto(LibraryManager.library, GodotGameEmbed);
