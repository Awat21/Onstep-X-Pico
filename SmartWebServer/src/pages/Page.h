// SWS standard page
#pragma once

#include "htmlHeaders.h"
#include "htmlMessages.h"
#include "htmlScripts.h"

#define PAGE_CONTROLLER 0
#define PAGE_MOUNT 1
#define PAGE_ROTATOR 2
#define PAGE_FOCUSER 3
#define PAGE_AUXILIARY 4
#define PAGE_ENCODERS 5
#define PAGE_NETWORK 6

void pageHeader(int selected);

const char html_onstep_header_begin[] PROGMEM = "<header class='hdr'><div class='hdr-row'><span class='brand'>";
const char html_onstep_header_title[] PROGMEM = "</span><span class='meta'>Smart Web Server ";
const char html_onstep_header_links[] PROGMEM = ")</span></div><nav class='tabs'>";
const char html_onstep_header_end[] PROGMEM = "</nav></header>\n";
const char html_onstep_page_begin[] PROGMEM = "<div class='b'>\n";

const char html_links_idx_begin[] PROGMEM = "<a href='/index.htm' ";
const char html_links_idx_end[] PROGMEM = ">" L_CONTROLLER "</a>";

const char html_links_mnt_begin[] PROGMEM = "<a href='/mount.htm' ";
const char html_links_mnt_end[] PROGMEM = ">" L_PAGE_MOUNT "</a>";

const char html_links_rot_begin[] PROGMEM = "<a href='/rotator.htm' ";
const char html_links_rot_end[] PROGMEM = ">" L_ROTATOR "</a>";

const char html_links_foc_begin[] PROGMEM = "<a href='/focuser.htm' ";
const char html_links_foc_end[] PROGMEM = ">" L_FOCUSER "</a>";

const char html_links_aux_begin[] PROGMEM = "<a href='/auxiliary.htm' ";
const char html_links_aux_end[] PROGMEM = ">" L_PAGE_AUXILIARY "</a>";

const char html_links_enc_begin[] PROGMEM = "<a href='/enc.htm' ";
const char html_links_enc_end[] PROGMEM = ">" L_PAGE_ENCODERS "</a>";

const char html_links_net_begin[] PROGMEM = "<a href='/net.htm' ";
const char html_links_net_end[] PROGMEM = ">" L_PAGE_NETWORK "</a>";

const char html_links_selected[] PROGMEM = " class='on' ";
