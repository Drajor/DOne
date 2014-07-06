/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#ifndef COMMAND_H
#define COMMAND_H

#include "../common/seperator.h"
#include "../common/EQStream.h"
#include "client.h"

#define	COMMAND_CHAR	'#'
#define CMDALIASES	5

typedef void (*CmdFuncPtr)(Client* ,const Seperator *);

// this is a command list item
/*struct cl_struct
{
	char *command[CMDALIASES];			// the command(s)
	char *desc;					// description of command
	CmdFuncPtr function;				// the function to call
	int access;					// the required 'status' level

	struct cl_struct *next;			// linked list
};

extern struct cl_struct *commandlist;		// the head of the list
*/

typedef struct {
	const char *command[CMDALIASES];			// the command(s)
	int access;
	const char *desc;		// description of command
	CmdFuncPtr function;	//null means perl function
} CommandRecord;

extern int (*command_dispatch)(Client* ,char const*);
extern int commandcount;			// number of commands loaded

// the command system:
int command_init(void);
void command_deinit(void);
int command_add(const char *command_string, const char *desc, int access, CmdFuncPtr function);
int command_notavail(Client* pClient, const char *message);
int command_realdispatch(Client* pClient, char const *message);
void command_logcommand(Client* pClient, const char *message);

//commands
void command_resetaa(Client* pClient,const Seperator* pSeperator);
void command_bind(Client* pClient,const Seperator* pSeperator);
void command_sendop(Client* pClient, const Seperator* pSeperator);
void command_optest(Client* pClient, const Seperator* pSeperator);
void command_setstat(Client* pClient, const Seperator* pSeperator);
void command_incstat(Client* pClient, const Seperator* pSeperator);
void command_help(Client* pClient, const Seperator* pSeperator);
void command_version(Client* pClient, const Seperator* pSeperator);
void command_setfaction(Client* pClient, const Seperator* pSeperator);
void command_serversidename(Client* pClient, const Seperator* pSeperator);
void command_testspawnkill(Client* pClient, const Seperator* pSeperator);
void command_testspawn(Client* pClient, const Seperator* pSeperator);
void command_wc(Client* pClient, const Seperator* pSeperator);
void command_numauths(Client* pClient, const Seperator* pSeperator);
void command_setanim(Client* pClient, const Seperator* pSeperator);
void command_connectworldserver(Client* pClient, const Seperator* pSeperator);
void command_serverinfo(Client* pClient, const Seperator* pSeperator);
void command_crashtest(Client* pClient, const Seperator* pSeperator);
void command_getvariable(Client* pClient, const Seperator* pSeperator);
void command_chat(Client* pClient, const Seperator* pSeperator);
void command_showpetspell(Client* pClient, const Seperator* pSeperator);
void command_ipc(Client* pClient, const Seperator* pSeperator);
void command_npcloot(Client* pClient, const Seperator* pSeperator);
void command_log(Client* pClient, const Seperator* pSeperator);
void command_gm(Client* pClient, const Seperator* pSeperator);
void command_summon(Client* pClient, const Seperator* pSeperator);
void command_zone(Client* pClient, const Seperator* pSeperator);
void command_zone_instance(Client* pClient, const Seperator* pSeperator);
void command_peqzone(Client* pClient, const Seperator* pSeperator);
void command_showbuffs(Client* pClient, const Seperator* pSeperator);
void command_movechar(Client* pClient, const Seperator* pSeperator);
void command_viewpetition(Client* pClient, const Seperator* pSeperator);
void command_petitioninfo(Client* pClient, const Seperator* pSeperator);
void command_delpetition(Client* pClient, const Seperator* pSeperator);
void command_date(Client* pClient, const Seperator* pSeperator);
void command_timezone(Client* pClient, const Seperator* pSeperator);
void command_synctod(Client* pClient, const Seperator* pSeperator);
void command_invul(Client* pClient, const Seperator* pSeperator);
void command_hideme(Client* pClient, const Seperator* pSeperator);
void command_emote(Client* pClient, const Seperator* pSeperator);
void command_fov(Client* pClient, const Seperator* pSeperator);
void command_manastat(Client* pClient, const Seperator* pSeperator);
void command_npcstats(Client* pClient, const Seperator* pSeperator);
void command_zclip(Client* pClient, const Seperator* pSeperator);
void command_npccast(Client* pClient, const Seperator* pSeperator);
void command_zstats(Client* pClient, const Seperator* pSeperator);
void command_permaclass(Client* pClient, const Seperator* pSeperator);
void command_permarace(Client* pClient, const Seperator* pSeperator);
void command_permagender(Client* pClient, const Seperator* pSeperator);
void command_weather(Client* pClient, const Seperator* pSeperator);
void command_zheader(Client* pClient, const Seperator* pSeperator);
void command_zsky(Client* pClient, const Seperator* pSeperator);
void command_zcolor(Client* pClient, const Seperator* pSeperator);
void command_spon(Client* pClient, const Seperator* pSeperator);
void command_spoff(Client* pClient, const Seperator* pSeperator);
void command_itemtest(Client* pClient, const Seperator* pSeperator);
void command_gassign(Client* pClient, const Seperator* pSeperator);
void command_ai(Client* pClient, const Seperator* pSeperator);
void command_worldshutdown(Client* pClient, const Seperator* pSeperator);
void command_sendzonespawns(Client* pClient, const Seperator* pSeperator);
void command_zsave(Client* pClient, const Seperator* pSeperator);
void command_dbspawn2(Client* pClient, const Seperator* pSeperator);
void command_copychar(Client* pClient, const Seperator* pSeperator);
void command_shutdown(Client* pClient, const Seperator* pSeperator);
void command_delacct(Client* pClient, const Seperator* pSeperator);
void command_setpass(Client* pClient, const Seperator* pSeperator);
void command_setlsinfo(Client* pClient, const Seperator* pSeperator);
void command_grid(Client* pClient, const Seperator* pSeperator);
void command_wp(Client* pClient, const Seperator* pSeperator);
void command_size(Client* pClient, const Seperator* pSeperator);
void command_mana(Client* pClient, const Seperator* pSeperator);
void command_flymode(Client* pClient, const Seperator* pSeperator);
void command_showskills(Client* pClient, const Seperator* pSeperator);
void command_findspell(Client* pClient, const Seperator* pSeperator);
void command_castspell(Client* pClient, const Seperator* pSeperator);
void command_setlanguage(Client* pClient, const Seperator* pSeperator);
void command_setskill(Client* pClient, const Seperator* pSeperator);
void command_setskillall(Client* pClient, const Seperator* pSeperator);
void command_race(Client* pClient, const Seperator* pSeperator);
void command_gender(Client* pClient, const Seperator* pSeperator);
void command_makepet(Client* pClient, const Seperator* pSeperator);
void command_level(Client* pClient, const Seperator* pSeperator);
void command_spawn(Client* pClient, const Seperator* pSeperator);
void command_texture(Client* pClient, const Seperator* pSeperator);
void command_npctypespawn(Client* pClient, const Seperator* pSeperator);
void command_heal(Client* pClient, const Seperator* pSeperator);
void command_appearance(Client* pClient, const Seperator* pSeperator);
void command_charbackup(Client* pClient, const Seperator* pSeperator);
void command_nukeitem(Client* pClient, const Seperator* pSeperator);
void command_peekinv(Client* pClient, const Seperator* pSeperator);
void command_findnpctype(Client* pClient, const Seperator* pSeperator);
void command_findzone(Client* pClient, const Seperator* pSeperator);
void command_viewnpctype(Client* pClient, const Seperator* pSeperator);
void command_reloadqst(Client* pClient, const Seperator* pSeperator);
void command_reloadworld(Client* pClient, const Seperator* pSeperator);
void command_reloadzps(Client* pClient, const Seperator* pSeperator);
void command_zoneshutdown(Client* pClient, const Seperator* pSeperator);
void command_zonebootup(Client* pClient, const Seperator* pSeperator);
void command_kick(Client* pClient, const Seperator* pSeperator);
void command_attack(Client* pClient, const Seperator* pSeperator);
void command_lock(Client* pClient, const Seperator* pSeperator);
void command_unlock(Client* pClient, const Seperator* pSeperator);
void command_motd(Client* pClient, const Seperator* pSeperator);
void command_listpetition(Client* pClient, const Seperator* pSeperator);
void command_equipitem(Client* pClient, const Seperator* pSeperator);
void command_zonelock(Client* pClient, const Seperator* pSeperator);
void command_corpse(Client* pClient, const Seperator* pSeperator);
void command_fixmob(Client* pClient, const Seperator* pSeperator);
void command_gmspeed(Client* pClient, const Seperator* pSeperator);
void command_title(Client* pClient, const Seperator* pSeperator);
void command_titlesuffix(Client* pClient, const Seperator* pSeperator);
void command_spellinfo(Client* pClient, const Seperator* pSeperator);
void command_lastname(Client* pClient, const Seperator* pSeperator);
void command_memspell(Client* pClient, const Seperator* pSeperator);
void command_save(Client* pClient, const Seperator* pSeperator);
void command_showstats(Client* pClient, const Seperator* pSeperator);
void command_mystats(Client* pClient, const Seperator* pSeperator);
void command_myskills(Client* pClient, const Seperator* pSeperator);
void command_depop(Client* pClient, const Seperator* pSeperator);
void command_depopzone(Client* pClient, const Seperator* pSeperator);
void command_repop(Client* pClient, const Seperator* pSeperator);
void command_spawnstatus(Client* pClient, const Seperator* pSeperator);
void command_nukebuffs(Client* pClient, const Seperator* pSeperator);
void command_zuwcoords(Client* pClient, const Seperator* pSeperator);
void command_zunderworld(Client* pClient, const Seperator* pSeperator);
void command_zsafecoords(Client* pClient, const Seperator* pSeperator);
void command_freeze(Client* pClient, const Seperator* pSeperator);
void command_unfreeze(Client* pClient, const Seperator* pSeperator);
void command_pvp(Client* pClient, const Seperator* pSeperator);
void command_setxp(Client* pClient, const Seperator* pSeperator);
void command_setpvppoints(Client* pClient, const Seperator* pSeperator);
void command_name(Client* pClient, const Seperator* pSeperator);
void command_tempname(Client* pClient, const Seperator* pSeperator);
void command_npcspecialattk(Client* pClient, const Seperator* pSeperator);
void command_kill(Client* pClient, const Seperator* pSeperator);
void command_haste(Client* pClient, const Seperator* pSeperator);
void command_damage(Client* pClient, const Seperator* pSeperator);
void command_zonespawn(Client* pClient, const Seperator* pSeperator);
void command_npcspawn(Client* pClient, const Seperator* pSeperator);
void command_spawnfix(Client* pClient, const Seperator* pSeperator);
void command_loc(Client* pClient, const Seperator* pSeperator);
void command_goto(Client* pClient, const Seperator* pSeperator);
#ifdef BUGTRACK
void command_bug(Client* pClient, const Seperator* pSeperator);
#endif
void command_iteminfo(Client* pClient, const Seperator* pSeperator);
void command_uptime(Client* pClient, const Seperator* pSeperator);
void command_flag(Client* pClient, const Seperator* pSeperator);
void command_time(Client* pClient, const Seperator* pSeperator);
void command_guild(Client* pClient, const Seperator* pSeperator);
bool helper_guild_edit(Client* pClient, uint32 dbid, uint32 eqid, uint8 rank, const char* what, const char* value);
void command_zonestatus(Client* pClient, const Seperator* pSeperator);
void command_manaburn(Client* pClient, const Seperator* pSeperator);
void command_viewmessage(Client* pClient, const Seperator* pSeperator);
void command_doanim(Client* pClient, const Seperator* pSeperator);
void command_randomfeatures(Client* pClient, const Seperator* pSeperator);
void command_face(Client* pClient, const Seperator* pSeperator);
void command_helm(Client* pClient, const Seperator* pSeperator);
void command_hair(Client* pClient, const Seperator* pSeperator);
void command_haircolor(Client* pClient, const Seperator* pSeperator);
void command_beard(Client* pClient, const Seperator* pSeperator);
void command_beardcolor(Client* pClient, const Seperator* pSeperator);
void command_tattoo(Client* pClient, const Seperator* pSeperator);
void command_heritage(Client* pClient, const Seperator* pSeperator);
void command_details(Client* pClient, const Seperator* pSeperator);
void command_scribespells(Client* pClient, const Seperator* pSeperator);
void command_unscribespells(Client* pClient, const Seperator* pSeperator);
void command_wpinfo(Client* pClient, const Seperator* pSeperator);
void command_wpadd(Client* pClient, const Seperator* pSeperator);
void command_interrupt(Client* pClient, const Seperator* pSeperator);
void command_d1(Client* pClient, const Seperator* pSeperator);
void command_summonitem(Client* pClient, const Seperator* pSeperator);
void command_giveitem(Client* pClient, const Seperator* pSeperator);
void command_givemoney(Client* pClient, const Seperator* pSeperator);
void command_itemsearch(Client* pClient, const Seperator* pSeperator);
void command_setaaxp(Client* pClient, const Seperator* pSeperator);
void command_setaapts(Client* pClient, const Seperator* pSeperator);
void command_setcrystals(Client* pClient, const Seperator* pSeperator);
void command_stun(Client* pClient, const Seperator* pSeperator);
void command_ban(Client* pClient, const Seperator* pSeperator);
void command_suspend(Client* pClient, const Seperator* pSeperator);
void command_ipban(Client* pClient, const Seperator* pSeperator);
void command_oocmute(Client* pClient, const Seperator* pSeperator);
void command_revoke(Client* pClient, const Seperator* pSeperator);
void command_checklos(Client* pClient, const Seperator* pSeperator);
void command_set_adventure_points(Client* pClient, const Seperator* pSeperator);
void command_npcsay(Client* pClient, const Seperator* pSeperator);
void command_npcshout(Client* pClient, const Seperator* pSeperator);
void command_npcemote(Client* pClient, const Seperator* pSeperator);
void command_npcedit(Client* pClient, const Seperator* pSeperator);
void command_timers(Client* pClient, const Seperator* pSeperator);
void command_undye(Client* pClient, const Seperator* pSeperator);
void command_undyeme(Client* pClient, const Seperator* pSeperator);
void command_hp(Client* pClient, const Seperator* pSeperator);
void command_logs(Client* pClient, const Seperator* pSeperator);
void command_nologs(Client* pClient, const Seperator* pSeperator);
void command_logsql(Client* pClient, const Seperator* pSeperator);
void command_qglobal(Client* pClient, const Seperator* pSeperator);
void command_path(Client* pClient, const Seperator* pSeperator);
void command_opcode(Client* pClient, const Seperator* pSeperator);
void command_aggro(Client* pClient, const Seperator* pSeperator);
void command_hatelist(Client* pClient, const Seperator* pSeperator);
void command_aggrozone(Client* pClient, const Seperator* pSeperator);
void command_reloadstatic(Client* pClient, const Seperator* pSeperator);
void command_flags(Client* pClient, const Seperator* pSeperator);
void command_flagedit(Client* pClient, const Seperator* pSeperator);
void command_mlog(Client* pClient, const Seperator* pSeperator);
void command_guildcreate(Client* pClient, const Seperator* pSeperator);
void command_guildapprove(Client* pClient, const Seperator* pSeperator);
void command_guildlist(Client* pClient, const Seperator* pSeperator);
void command_task(Client* pClient, const Seperator* pSeperator);
void command_reloadtitles(Client* pClient, const Seperator* pSeperator);
void command_altactivate(Client* pClient, const Seperator* pSeperator);
void command_refundaa(Client* pClient, const Seperator* pSeperator);
void command_traindisc(Client* pClient, const Seperator* pSeperator);
void command_deletegraveyard(Client* pClient, const Seperator* pSeperator);
void command_setgraveyard(Client* pClient, const Seperator* pSeperator);
void command_getplayerburriedcorpsecount(Client* pClient, const Seperator* pSeperator);
void command_summonburriedplayercorpse(Client* pClient, const Seperator* pSeperator);
void command_unscribespell(Client* pClient, const Seperator* pSeperator);
void command_scribespell(Client* pClient, const Seperator* pSeperator);
void command_refreshgroup(Client* pClient, const Seperator* pSeperator);
void command_advnpcspawn(Client* pClient, const Seperator* pSeperator);
void command_modifynpcstat(Client* pClient, const Seperator* pSeperator);
void command_instance(Client* pClient, const Seperator* pSeperator);
void command_setstartzone(Client* pClient, const Seperator* pSeperator);
void command_netstats(Client* pClient, const Seperator* pSeperator);
void command_object(Client* pClient, const Seperator* pSeperator);
void command_raidloot(Client* pClient, const Seperator* pSeperator);
void command_globalview(Client* pClient, const Seperator* pSeperator);
void command_emoteview(Client* pClient, const Seperator* pSeperator);
void command_reloademote(Client* pClient, const Seperator* pSeperator);
void command_emotesearch(Client* pClient, const Seperator* pSeperator);
void command_distance(Client* pClient, const Seperator* pSeperator);
void command_cvs(Client* pClient, const Seperator* pSeperator);
void command_max_all_skills(Client* pClient, const Seperator* pSeperator);
void command_showbonusstats(Client* pClient, const Seperator* pSeperator);
void command_reloadallrules(Client* pClient, const Seperator* pSeperator);
void command_reloadworldrules(Client* pClient, const Seperator* pSeperator);
void command_reloadlevelmods(Client* pClient, const Seperator* pSeperator);
void command_camerashake(Client* pClient, const Seperator* pSeperator);
void command_disarmtrap(Client* pClient, const Seperator* pSeperator);
void command_sensetrap(Client* pClient, const Seperator* pSeperator);
void command_picklock(Client* pClient, const Seperator* pSeperator);
void command_mysql(Client* pClient, const Seperator* pSeperator);
void command_xtargets(Client* pClient, const Seperator* pSeperator);
void command_zopp(Client* pClient, const Seperator* pSeperator);
void command_augmentitem(Client* pClient, const Seperator* pSeperator);
void command_questerrors(Client* pClient, const Seperator* pSeperator);
void command_enablerecipe(Client* pClient, const Seperator* pSeperator);
void command_disablerecipe(Client* pClient, const Seperator* pSeperator);
void command_showspellslist(Client* pClient, const Seperator* pSeperator);
void command_npctype_cache(Client* pClient, const Seperator* pSeperator);
void command_merchantopenshop(Client* pClient, const Seperator* pSeperator);
void command_merchantcloseshop(Client* pClient, const Seperator* pSeperator);

#ifdef EQPROFILE
void command_profiledump(Client* pClient, const Seperator* pSeperator);
void command_profilereset(Client* pClient, const Seperator* pSeperator);
#endif

#ifdef PACKET_PROFILER
void command_packetprofile(Client* pClient, const Seperator* pSeperator);
#endif

#endif

