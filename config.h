/* See LICENSE file for copyright and license details. */

/*   Display modes of the tab bar: never shown, always shown, shown only in  */
/*   monocle mode in presence of several windows.                            */
/*   A mode can be disabled by moving it after the showtab_nmodes end marker */
#include <X11/XF86keysym.h>

enum showtab_modes { showtab_never, showtab_auto, showtab_nmodes, showtab_always};
static const int showtab            = showtab_auto; /* Default tab bar show mode  */
static const Bool toptab            = False;         /* False means bottom tab bar */

/* appearance */
static const char *fonts[] = {
   "DejaVuSansMono Nerd Font:style=Book:size=12",
   "Noto Color Emoji"
};

static const char dmenufont[]       = "DejaVuSansMono Nerd Font:style=Book:size=10,Noto Color Emoji";
static const char normbordercolor[] = "#282828";
static const char normbgcolor[]     = "#282828";
static const char normfgcolor[]     = "#eeeeee";
static const char selbordercolor[]  = "#93a1a1";
static const char selbgcolor[]      = "#93a1a1";
static const char selfgcolor[]      = "#282828";
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */


static const char *upvol[]   =   { "/usr/bin/pactl", "set-sink-volume", "0", "+5%",     NULL };
static const char *downvol[] =   { "/usr/bin/pactl", "set-sink-volume", "0", "-5%",     NULL };
static const char *mutevol[] =   { "/usr/bin/pactl", "set-sink-mute",   "0", "toggle",  NULL };

static const char *lightup[] =   { "/usr/bin/xbacklight", "-inc",   "5", NULL };
static const char *lightdown[] = { "/usr/bin/xbacklight", "-dec",   "5", NULL };

static const char *musicplay[] = { "/usr/bin/playerctl", "play-pause", NULL};
static const char *musicstop[] = { "/usr/bin/playerctl", "stop", NULL};
static const char *musicprev[] = { "/usr/bin/playerctl", "previous", NULL};
static const char *musicnext[] = { "/usr/bin/playerctl", "next", NULL};


static const char *full_screenshot[] = { "/bin/bash", "-c", "escrotum ~/Pictures/Screenshots/$(date +%s).png && notify-send 'Screenshot saved'", NULL };
static const char *sel_screenshot[] = { "/usr/bin/escrotum", "-s", "-C", NULL };

static const char *favourites_apps[] = { "/bin/sh", "-c", " ~/bin/dmenu_favourites ~/.config/favourites/apps", NULL };
static const char *favourites_sett[] = { "/bin/sh", "-c", " ~/bin/dmenu_favourites ~/.config/favourites/settings", NULL };
static const char *favourites_exit[] = { "/bin/sh", "-c", " ~/bin/dmenu_favourites ~/.config/favourites/exit", NULL };

static char dmenumon[2] = "0";
static const char *clipmenu[] = { "/usr/bin/clipmenu", NULL };
static const char *dmenu_emoju[] = { "/bin/sh", "-c", " ~/bin/dmenu-emoji", NULL };
static const char *bitwarden_dmenu[] = { "/bin/sh", "-c", " ~/bin/bitwarden_dmenu", NULL };
static const char *favourites_me[] = { "/bin/sh", "-c", " ~/bin/dmenu_favourites ~/.config/favourites/me", NULL };
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *j4cmd[] = {"/usr/local/bin/j4-dmenu-desktop"};
static const char *termcmd[]  = { "st", NULL };


/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor  hook*/
	/* class      instance         title      tags mask     isfloating     isterminal      noswallow    monitor    hook */
	{ NULL,       NULL,            NULL,       0,            0,            0,              0,           -1,        all_windows_hook },
	{ "Gimp",     NULL,            NULL,       0,            1,            0,              0,           -1,        NULL },
	{ NULL,       "floating",      NULL,       0,            1,            0,              0,           -1,        NULL },
	{ "st",       NULL,            NULL,       0,            0,            1,              1,           -1,        NULL },
};


static const IPCHook ipchooks[] = {
	{ "next_sfc",  next_sfc },
	{ "swallow", set_doswallow },
	{ "noswallow", unset_doswallow }
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

#include "gaplessgrid.c"
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "tile",           tile },    /* first entry is default */
	{ "float",          NULL },    /* no layout function means floating behavior */
	{ "monocle",        monocle },
	{ "centeredmaster", centeredmaster },
	{ "grid",           gaplessgrid },
};
/* default layout per tags */
/* The first element is for all-tag view, following i-th element corresponds to */
/* tags[i]. Layout is referred using the layouts array index.*/
static int def_layouts[1 + LENGTH(tags)]  = {4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2};


/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */


static Key keys[] = {
	/* modifier                     key        function        argument */
	// Volume control
	{ 0,              XF86XK_AudioLowerVolume, spawn,          {.v = downvol } },
	{ 0,                     XF86XK_AudioMute, spawn,          {.v = mutevol } },
	{ 0,              XF86XK_AudioRaiseVolume, spawn,          {.v = upvol   } },

	// Brightness control
	{ 0,           XF86XK_MonBrightnessUp,     spawn,          {.v = lightup   } },
	{ 0,           XF86XK_MonBrightnessDown,   spawn,          {.v = lightdown   } },

	// Media control
    { 0,           XF86XK_AudioPlay,           spawn,          {.v = musicplay  } },
	{ 0,           XF86XK_AudioStop,           spawn,          {.v = musicstop  } },
	{ 0,           XF86XK_AudioNext,           spawn,          {.v = musicnext  } },
	{ 0,           XF86XK_AudioPrev,           spawn,          {.v = musicprev  } },
	
	// Spawn shortcuts
	{ 0,                            XK_Print,  spawn,          {.v = sel_screenshot   } },
	{ MODKEY,                       XK_v,      spawn,          {.v = clipmenu   } },
	{ ShiftMask,                    XK_Print,  spawn,          {.v = full_screenshot   } },
	{ MODKEY,                       XK_o,      spawn,      	   {.v = favourites_apps} },
	{ MODKEY,                       XK_s,      spawn,      	   {.v = favourites_sett} },
	{ MODKEY,                       XK_Escape, spawn,      	   {.v = favourites_exit} },
	{ MODKEY,                       XK_e,      spawn,      	   {.v = dmenu_emoju} },
	{ MODKEY|ShiftMask,             XK_p,      spawn,      	   {.v = bitwarden_dmenu} },
	{ MODKEY|ShiftMask,             XK_m,      spawn,      	   {.v = favourites_me} },
	{ MODKEY|ShiftMask,             XK_space,  spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_space,  spawn,          {.v = j4cmd } },

	// Visibility controls
	{ MODKEY,                       XK_f,      togglefullscreen,{0} },
	{ MODKEY,                       XK_x,      togglehidden,   {0} },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefloating, {0} },

	// Layouts switch
	{ MODKEY,                       XK_g,      setlayout,      {.v = &layouts[4] } },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_c,      setlayout,      {.v = &layouts[3]} },

	// Layout controls
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_p,      zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },

	// Kill client
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },

	// Multi monitor controls
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },

	// Desktop controls
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	TAGKEYS(                        XK_0,                      9)
	
	{ MODKEY|ShiftMask,             XK_e,      quit,           {0} },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkTabBar,            0,              Button1,        focuswin,       {0} },
	{ ClkStatusText,        0,              Button4,        spawn,          {.v=upvol} },
	{ ClkStatusText,        0,              Button5,        spawn,        {.v=downvol} },
	{ ClkStatusText,        ShiftMask,      Button4,        spawn,        {.v=lightup} },
	{ ClkStatusText,        ShiftMask,      Button5,        spawn,      {.v=lightdown} },
};

