#pragma once

#include "Pages.common.h"

#ifndef COLOR_BODY_BACKGROUND
#define COLOR_BODY_BACKGROUND "#0B0607"
#endif

#ifndef COLOR_FOREGROUND
#define COLOR_FOREGROUND "#B48A8A"
#endif
#ifndef COLOR_LIGHT_FOREGROUND
#define COLOR_LIGHT_FOREGROUND "#C9A0A0"
#endif
#ifndef COLOR_ALERT
#define COLOR_ALERT "#FF7043"
#endif
#ifndef COLOR_BACKGROUND
#define COLOR_BACKGROUND "#150A0B"
#endif
#ifndef COLOR_LIGHT_BACKGROUND
#define COLOR_LIGHT_BACKGROUND "#1D0F10"
#endif
#ifndef COLOR_BORDER
#define COLOR_BORDER "#3A1719"
#endif

#ifndef COLOR_LINK
#define COLOR_LINK "#C08080"
#endif
#ifndef COLOR_LINK_BACKGROUND
#define COLOR_LINK_BACKGROUND "#1D0F10"
#endif
#ifndef COLOR_LINK_SELECTED_BACKGROUND
#define COLOR_LINK_SELECTED_BACKGROUND "#3A1416"
#endif
#ifndef COLOR_LINK_BORDER
#define COLOR_LINK_BORDER "#D03232"
#endif

#ifndef COLOR_CONTROL
#define COLOR_CONTROL "#D03232"
#endif
#ifndef COLOR_CONTROL_BACKGROUND
#define COLOR_CONTROL_BACKGROUND "#1D0F10"
#endif
#ifndef COLOR_CONTROL_DISABLED
#define COLOR_CONTROL_DISABLED "#5A3A3B"
#endif
#ifndef COLOR_CONTROL_DISABLED_BACKGROUND
#define COLOR_CONTROL_DISABLED_BACKGROUND "#140A0B"
#endif

#ifndef COLOR_COLLAPSIBLE
#define COLOR_COLLAPSIBLE "#B48A8A"
#endif
#ifndef COLOR_COLLAPSIBLE_BACKGROUND
#define COLOR_COLLAPSIBLE_BACKGROUND "#1D0F10"
#endif
#ifndef COLOR_COLLAPSIBLE_ACTIVE_BACKGROUND
#define COLOR_COLLAPSIBLE_ACTIVE_BACKGROUND "#2A1416"
#endif
#ifndef COLOR_COLLAPSIBLE_CONTENT_BACKGROUND
#define COLOR_COLLAPSIBLE_CONTENT_BACKGROUND "#110809"
#endif

#ifndef COLOR_SERVO_BACKGROUND_1
#define COLOR_SERVO_BACKGROUND_1 "#1D0F10"
#endif
#ifndef COLOR_SERVO_BACKGROUND_2
#define COLOR_SERVO_BACKGROUND_2 "#3A1416"
#endif
#ifndef COLOR_SERVO_BACKGROUND_3
#define COLOR_SERVO_BACKGROUND_3 "#A02222"
#endif
#ifndef COLOR_SERVO_PEN_1
#define COLOR_SERVO_PEN_1 "#2A1416"
#endif
#ifndef COLOR_SERVO_PEN_2
#define COLOR_SERVO_PEN_2 "#3E1D1F"
#endif
#ifndef COLOR_SERVO_PEN_3
#define COLOR_SERVO_PEN_3 "#5E2C2E"
#endif
#ifndef COLOR_SERVO_PEN_4
#define COLOR_SERVO_PEN_4 "#C9A0A0"
#endif

const char html_head_begin[] PROGMEM = "<!DOCTYPE HTML>\n"
                                  "<html>\n"
                                  "<head>\n"
                                  "<meta charset='utf-8'/>\n"
                                  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n";
const char html_headerPec[] PROGMEM = "<meta http-equiv=\"refresh\" content=\"5; URL=/pec.htm\">\n";
const char html_headerIdx[] PROGMEM = "<meta http-equiv=\"refresh\" content=\"5; URL=/index.htm\">\n";
const char html_head_end[] PROGMEM = "</head>\n";
const char html_body_begin[] PROGMEM = "<body>\n";
const char html_page_and_body_end[] PROGMEM = "</div></body></html>\n";

const char html_main_css_begin[] PROGMEM = 
"<STYLE>";

// night mode palette, everything is a dim red to protect dark adaptation
const char html_main_css_core[] PROGMEM =
":root { --bg: " COLOR_BODY_BACKGROUND "; --fg: " COLOR_FOREGROUND "; --fg-hi: " COLOR_LIGHT_FOREGROUND "; --alert: " COLOR_ALERT ";"
"--card: " COLOR_BACKGROUND "; --card-hi: " COLOR_LIGHT_BACKGROUND "; --line: " COLOR_BORDER "; --link: " COLOR_LINK ";"
"--accent: " COLOR_CONTROL "; --sel: " COLOR_LINK_SELECTED_BACKGROUND "; --off: " COLOR_CONTROL_DISABLED "; }"
"* { box-sizing: border-box; } "
"body { margin: 0; background: var(--bg); color: var(--fg); font: 15px/1.5 'Segoe UI', system-ui, -apple-system, Roboto, sans-serif; } "
".clear { clear: both; } .a { background-color: var(--card); } "
".hdr { position: sticky; top: 0; z-index: 10; background: var(--card); border-bottom: 1px solid var(--line); padding: 0 24px; } "
".hdr-row { display: flex; align-items: baseline; justify-content: space-between; gap: 16px; padding: 12px 0 6px; } "
".brand { font-size: 20px; font-weight: 600; color: var(--fg-hi); letter-spacing: .02em; } "
".meta { font-size: 12px; color: var(--fg); opacity: .7; } "
".tabs { display: flex; flex-wrap: wrap; gap: 4px; } "
".tabs a:link, .tabs a:visited { background: none; border: none; border-bottom: 2px solid transparent; border-radius: 0; color: var(--fg); padding: 8px 14px; margin: 0; } "
".tabs a:hover { background: var(--card-hi); color: var(--fg-hi); } "
".tabs a.on { color: var(--fg-hi); border-bottom-color: var(--accent); } "
".t { margin: 0; padding: 12px 24px; background: var(--card); border-bottom: 1px solid var(--line); } "
".b { max-width: 1440px; margin: 0 auto; padding: 16px 16px 32px; } "
"hr { border: none; height: 1px; background: var(--line); margin: 10px 0; } "
"h1 { text-align: right; } .c { color: var(--accent); font-weight: 600; } .y { color: var(--alert); font-weight: 600; } "
"a:link, a:visited { background: var(--card-hi); color: var(--link); border: 1px solid var(--line); border-radius: 6px; padding: 5px 12px; text-align: center; text-decoration: none; display: inline-block; } "
"a:hover, a:active { border-color: var(--accent); color: var(--fg-hi); } "
"input, select, textarea { font: inherit; font-weight: 600; color: var(--fg-hi); background: var(--bg); border: 1px solid var(--line); border-radius: 6px; padding: 3px 6px; margin: 3px; text-align: center; width: 5em; } "
"select { width: auto; text-align: left; } "
"input[type=range] { border: none; background: none; padding: 0; accent-color: var(--accent); } "
"input:focus, select:focus, textarea:focus { outline: none; border-color: var(--accent); } "
"button { font: inherit; font-weight: 600; color: var(--accent); background: var(--card-hi); border: 1px solid var(--accent); border-radius: 8px; margin: 2px; padding: 4px 12px; cursor: pointer; } "
"button:hover { background: var(--sel); } button:active, button.sel { background: var(--accent); color: var(--bg); } "
"button:disabled { color: var(--off); border-color: var(--line); background: none; cursor: default; }";

const char html_main_css_no_select[] PROGMEM =
"body { -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; }";

// button groups, note btns_right is the left end of a group (rounded on the left, joined on the right)
const char html_main_css_buttons[] PROGMEM = 
".btns_left, .btns_mid, .btns_right { padding: 4px 9px; font-size: 13px; }"
".bbh.btns_left, .bbh.btns_mid, .bbh.btns_right { padding: 4px 16px; font-size: 15px; }"
".btns_left { margin-left: -1px; border-top-left-radius: 0px; border-bottom-left-radius: 0px; }"
".btns_mid { margin-left: -1px; margin-right: -1px; border-radius: 0px; }"
".btns_right { margin-right: -1px; border-top-right-radius: 0px; border-bottom-right-radius: 0px; }";

const char html_main_css_control[] PROGMEM = 
".b1 { float: left; background: var(--card); border: 1px solid var(--line); border-radius: 12px; text-align: center; margin: 8px; padding: 12px 16px 14px; } "
".b1 > hr:last-child { display: none; } "
".b1 > div:first-child { font-size: 12px; font-weight: 600; letter-spacing: .08em; text-transform: uppercase; color: var(--fg-hi); } "
".b1 > div:first-child .c { letter-spacing: normal; text-transform: none; } "
".b1 > div[style*='float: right'] { font-size: 12px; padding: 1px 10px; border-radius: 999px; background: var(--sel); } "
".gb { font-size: 22px; font-weight: 600; width: 64px; height: 56px; padding: 0; margin: 3px; border-radius: 14px; touch-action: none; -webkit-touch-callout: none; } "
".bb { font-weight: 600; font-size: 105%; } .bbh { font-weight: 600; height: 2.2em; min-width: 5em; touch-action: none; -webkit-touch-callout: none; }";

const char html_main_css_collapse[] PROGMEM =
".collapsible { background: var(--card-hi); color: var(--fg); cursor: pointer; padding: 7px 12px; width: 100%; border: 1px solid var(--line); border-radius: 8px; text-align: left; font-size: 13px; margin: 2px 0; } "
".collapsible::before { content: '\\25B8'; display: inline-block; width: 1.2em; color: var(--accent); } "
".collapsible.active::before { content: '\\25BE'; } "
".active, .collapsible:hover { background: var(--sel); color: var(--fg-hi); } "
".content { padding: 4px 14px; display: none; overflow: hidden; background: " COLOR_COLLAPSIBLE_CONTENT_BACKGROUND "; border: 1px solid var(--line); border-top: none; border-radius: 0 0 8px 8px; margin-top: -4px; }";

const char html_main_css_end[] PROGMEM = 
"</STYLE>";

// tile: begin takes title and width
const char html_tile_beg[] PROGMEM = "<div class='b1' style='width: %s; min-height: %s;'><div style='float: left;'>%s</div>\n";
// tile: begin for text takes title and width
const char html_tile_text_beg[] PROGMEM = "<div class='b1' style='text-align: left; width: %s; min-height: %s;'><div style='float: left;'>%s</div>\n";
// tile: adds status to first line
const char html_tile_status[] PROGMEM = "<div style='float: right; text-align: right;' id='%s' class='c'>%s</div><br />\n";
// tile: end
const char html_tile_end[] PROGMEM = "</div>\n";

// collapsable: begin takes title
const char html_collapsable_beg[] PROGMEM = 
"<div style='text-align: left;'> <button type='button' class='collapsible' style='vertical-align: bottom'>%s</button>"
"<div class='content' style='text-align: left;'><br />\n";
// collapsable: end
const char html_collapsable_end[] PROGMEM = "<br /></div></div>\n";

// form: begin takes webpage for get
const char html_form_begin[] PROGMEM = "<form method='get' action='/%s'>\n";
// form: end
const char html_form_end[] PROGMEM = "</form>\n";

// input: medium width numeric takes name (for get,) value, min, max, unit
const char html_ninput_wide[] PROGMEM =
"<input style='width: 4em;' type='number' name='%s' value='%s' min='%s' max='%s'>&nbsp;%s";
// input: narrow width numeric takes name (for get,) value, min, max, unit
const char html_ninput[] PROGMEM =
"<input style='width: 3.5em;' type='number' name='%s' value='%s' min='%s' max='%s'>&nbsp;%s";
