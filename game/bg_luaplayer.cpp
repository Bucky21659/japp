#if defined(PROJECT_GAME)
#include "g_local.h"
#include "g_admin.h"
#elif defined(PROJECT_CGAME)
#include "cg_local.h"
#endif
#include "bg_luainternal.h"

#ifdef JPLUA

#if defined(PROJECT_GAME)
extern qboolean SetEmote( gentity_t *ent, const emote_t *emote );
#endif

namespace JPLua {

	static const char PLAYER_META[] = "Player.meta";

	#if defined(PROJECT_GAME)
	static jpluaEntity_t *ents = g_entities;
	#elif defined(PROJECT_CGAME)
	static jpluaEntity_t *ents = cg_entities;
	#endif

	static playerState_t *GetPlayerstate( jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		return &ent->client->ps;
	#elif defined(PROJECT_CGAME)
		return ((int)(ent - ents) == cg.clientNum)
			? &cg.predictedPlayerState
			: nullptr;
	#endif
	}

	// Push a Player instance for a client number onto the stack
	void Player_CreateRef( lua_State *L, int num ) {
		luaPlayer_t *player = NULL;

	#if defined(PROJECT_GAME)
		if ( num < 0 || num >= level.maxclients || !g_entities[num].inuse ) {
	#elif defined(PROJECT_CGAME)
		if ( num < 0 || num >= cgs.maxclients || !cgs.clientinfo[num].infoValid ) {
	#endif
			lua_pushnil( L );
			return;
		}

		player = (luaPlayer_t *)lua_newuserdata( L, sizeof(luaPlayer_t) );
		player->clientNum = num;

		luaL_getmetatable( L, PLAYER_META );
		lua_setmetatable( L, -2 );
	}

	// Ensure the value at the specified index is a valid Player instance,
	// Return the instance if it is, otherwise return NULL.
	luaPlayer_t *CheckPlayer( lua_State *L, int idx ) {
		void *ud = luaL_checkudata( L, idx, PLAYER_META );
		luaL_argcheck( L, ud != NULL, 1, "'Player' expected" );
		return (luaPlayer_t *)ud;
	}

	int Player_GetMetaTable( lua_State *L ) {
		luaL_getmetatable( L, PLAYER_META );
		return 1;
	}

	#if defined(PROJECT_GAME)
	static int Player_GetAdminPrivs( lua_State *L, jpluaEntity_t *ent ) {
		if ( ent->client->pers.adminUser ) {
			lua_pushinteger( L, ent->client->pers.adminUser->privileges );
		}
		else {
			lua_pushinteger( L, ent->client->pers.tempprivs );
		}
		return 1;
	}
	#endif

	#if 0
	static const stringID_table_t ammo_strings[AMMO_MAX] = {
		ENUM2STRING(AMMO_NONE),
		ENUM2STRING(AMMO_FORCE),
		ENUM2STRING(AMMO_BLASTER),
		ENUM2STRING(AMMO_POWERCELL),
		ENUM2STRING(AMMO_METAL_BOLTS),
		ENUM2STRING(AMMO_ROCKETS),
		ENUM2STRING(AMMO_EMPLACED),
		ENUM2STRING(AMMO_THERMAL),
		ENUM2STRING(AMMO_TRIPMINE),
		ENUM2STRING(AMMO_DETPACK),
	};
	#endif

	static int Player_GetAmmo( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_newtable(L);
		int top = lua_gettop(L);
		for (int i = 1; i < WP_NUM_WEAPONS; i++) {
			lua_pushinteger(L, i);
			lua_pushinteger(L, ent->playerState->ammo[i]);
			lua_settable(L, top);
		}
	#elif defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
			lua_newtable( L );
			int top = lua_gettop( L );
			for ( int i = 1; i < WP_NUM_WEAPONS; i++ ) {
				lua_pushinteger( L, i );
				lua_pushinteger( L, cg.predictedPlayerState.ammo[i] );
				lua_settable( L, top );
			}
		}
		else {
			lua_pushnil( L );
		}
	#endif
		return 1;
	}

	static int Player_GetAngles( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		const vector3 *angles = &ent->client->ps.viewangles;
	#elif defined(PROJECT_CGAME)
		const vector3 *angles = ((int)(ent - ents) == cg.clientNum)
			? &cg.predictedPlayerState.viewangles
			: &ent->lerpAngles;
	#endif

		Vector_CreateRef( L, angles->x, angles->y, angles->z );
		return 1;
	}

	static int Player_GetArmor( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushinteger( L, ent->client->ps.stats[STAT_ARMOR] );
	#elif defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
			lua_pushinteger( L, cg.predictedPlayerState.stats[STAT_ARMOR] );
		}
		else {
			lua_pushnil( L );
		}
	#endif

		return 1;
	}

	static int Player_GetDuelingPartner( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );

	#if defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
	#endif
			if ( ps->duelInProgress ) {
				Player_CreateRef( L, ps->duelIndex );
			}
			else {
				lua_pushnil( L );
			}
	#if defined(PROJECT_CGAME)
		}
		else {
			lua_pushnil( L );
		}
	#endif

		return 1;
	}

	static int Player_GetEFlags( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );

		if ( ps ) {
			lua_pushinteger( L, ps->eFlags );
		}
		else {
			lua_pushnil( L );
		}

		return 1;
	}

	static int Player_GetEFlags2( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );

		if ( ps ) {
			lua_pushinteger( L, ps->eFlags2 );
		}
		else {
			lua_pushnil( L );
		}

		return 1;
	}

	#if defined(PROJECT_GAME)
	static int Player_GetFlags( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushinteger( L, ent->flags );
		return 1;
	}
	#endif

	static int Player_GetForce( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );

		if ( ps ) {
			lua_pushinteger( L, ps->fd.forcePower );
		}
		else {
			lua_pushnil( L );
		}
		return 1;
	}

	static int Player_GetHealth( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );
	#if defined(PROJECT_GAME)
		lua_pushinteger( L, ps->stats[STAT_HEALTH] );
	#elif defined(PROJECT_CGAME)
		if ( ps ) {
			lua_pushinteger( L, ps->stats[STAT_HEALTH] );
		}
		else {
			if ( cgs.clientinfo[(int)(ent - ents)].team == cg.predictedPlayerState.persistant[PERS_TEAM] ) {
				lua_pushinteger( L, ent->currentState.health );
			}
			else {
				lua_pushnil( L );
			}
		}
	#endif
		return 1;
	}

	static int Player_GetID( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushinteger( L, (int)(ent - ents) );
		return 1;
	}

	#if defined(PROJECT_GAME)
	static int Player_GetAdmin( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushboolean( L, ent->client->pers.adminUser ? 1 : 0 );
		return 1;
	}
	#endif

	static int Player_GetAlive( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		if ( ent->client->ps.stats[STAT_HEALTH] > 0
			&& !(ent->client->ps.eFlags & EF_DEAD)
			&& ent->client->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR
			&& ent->client->tempSpectate < level.time )
		{
			lua_pushboolean( L, 1 );
			return 1;
		}
	#elif defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
			if ( cg.predictedPlayerState.stats[STAT_HEALTH] > 0
				&& !(cg.predictedPlayerState.eFlags & EF_DEAD)
				&& cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_SPECTATOR )
			{
				lua_pushboolean( L, 1 );
				return 1;
			}
		}
		else if ( !(ent->currentState.eFlags & EF_DEAD)
			&& cgs.clientinfo[(int)(ent - ents)].team != TEAM_SPECTATOR )
		{
			lua_pushboolean( L, 1 );
			return 1;
		}
	#endif

		lua_pushboolean( L, 0 );
		return 1;
	}

	static int Player_GetBot( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushboolean( L, !!(ent->r.svFlags & SVF_BOT) );
	#elif defined(PROJECT_CGAME)
		lua_pushboolean( L, !!(cgs.clientinfo[(int)(ent - ents)].botSkill != -1) );
	#endif
		return 1;
	}

	static int Player_GetDueling( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushboolean( L, ent->client->ps.duelInProgress ? 1 : 0 );
	#else
		lua_pushboolean(L, cg.duelInProgress);
	#endif
		return 1;
	}

	#if defined(PROJECT_GAME)
	static int Player_GetEmpowered( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushboolean( L, ent->client->pers.adminData.empowered ? 1 : 0 );
		return 1;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_GetGhost( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushboolean( L, ent->client->pers.adminData.isGhost ? 1 : 0 );
		return 1;
	}
	#endif

	static int Player_GetHolstered( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		entityState_t *es = &ent->s;
	#elif defined(PROJECT_CGAME)
		entityState_t *es = &ent->currentState;
	#endif
		lua_pushboolean( L, es->saberHolstered );
		return 1;
	}

	static int Player_GetInAir( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		entityState_t *es = &ent->s;
	#elif defined(PROJECT_CGAME)
		entityState_t *es = &ent->currentState;
	#endif
		lua_pushboolean( L, es->groundEntityNum == ENTITYNUM_NONE );
		return 1;
	}

	#if defined(PROJECT_GAME)
	static int Player_GetMerced( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushboolean( L, ent->client->pers.adminData.merc ? 1 : 0 );
		return 1;
	}
	#endif

	static int Player_GetProtected( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		entityState_t *es = &ent->s;
	#elif defined(PROJECT_CGAME)
		entityState_t *es = &ent->currentState;
	#endif
		lua_pushboolean( L, (es->eFlags & EF_INVULNERABLE) ? 1 : 0 );
		return 1;
	}

	#if defined(PROJECT_GAME)
	static int Player_GetSilenced( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushboolean(L, ent->client->pers.adminData.silenced ? 1 : 0);
		return 0;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_GetSlept( lua_State *L, jpluaEntity_t *ent ) {
		lua_pushboolean( L, ent->client->pers.adminData.isSlept ? 1 : 0 );
		return 1;
	}
	#endif

	static int Player_GetUnderwater( lua_State *L, jpluaEntity_t *ent ) {
		qboolean underwater = qfalse;
	#if defined(PROJECT_GAME)
		if ( ent->waterlevel == 3 ) {
			underwater = qtrue;
		}
	#elif defined(PROJECT_CGAME)
		const vector3 *pos = ((int)(ent - ents) == cg.clientNum)
			? &cg.predictedPlayerState.origin
			: &ent->currentState.pos.trBase; // not cent->lerpOrigin?
		if ( CG_PointContents( pos, -1 ) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA) ) {
			underwater = qtrue;
		}
	#endif
		lua_pushboolean( L, underwater );
		return 1;
	}

	#if defined(PROJECT_CGAME)
	static int Player_GetLastPickup( lua_State *L, jpluaEntity_t *ent ) {
		if ( (int)(ent - ents) == cg.clientNum ) {
			lua_pushstring( L, CG_GetStringEdString( "SP_INGAME", bg_itemlist[cg.itemPickup].classname ) );
		}
		else {
			lua_pushnil( L );
		}
		return 1;
	}
	#endif

	static int Player_GetLegsAnim( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );
	#if defined(PROJECT_CGAME)
		entityState_t *es = &cg_entities[(int)(ent - ents)].currentState;
	#endif

	#if defined(PROJECT_CGAME)
		if ( ps ) {
	#endif
			lua_pushinteger( L, ps->legsAnim );
	#if defined(PROJECT_CGAME)
		}
		else {
			lua_pushinteger( L, es->legsAnim );
		}
	#endif

		return 1;
	}

	static int Player_GetLocation( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		char location[64];
		if ( Team_GetLocationMsg( ent, location, sizeof(location) ) ) {
			lua_pushstring( L, location );
		}
		else {
			lua_pushnil( L );
		}
	#elif defined(PROJECT_CGAME)
		lua_pushstring( L,
			CG_GetLocationString( CG_ConfigString( CS_LOCATIONS + cgs.clientinfo[(int)(ent - ents)].location ) )
		);
	#endif

		return 1;
	}

	static int Player_GetMaxAmmo( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_newtable( L );
		int top = lua_gettop( L );
		for ( int i = 1; i < WP_NUM_WEAPONS; i++ ) {
			lua_pushinteger( L, i );
			lua_pushinteger( L, ammoMax[weaponData[i].ammoIndex] );
			lua_settable( L, top );
		}
	#elif defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
			lua_newtable( L );
			int top = lua_gettop( L );
			for ( int i = 1; i < WP_NUM_WEAPONS; i++ ) {
				lua_pushinteger( L, i );
				lua_pushinteger( L, ammoMax[weaponData[i].ammoIndex] );
				lua_settable( L, top );
			}
		}
		else {
			lua_pushnil( L );
		}
	#endif

		return 1;
	}

	static int Player_GetName( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushstring( L, ent->client->pers.netname );
	#elif defined(PROJECT_CGAME)
		lua_pushstring( L, cgs.clientinfo[(int)(ent - ents)].name );
	#endif
		return 1;
	}

	static int Player_GetPosition( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		const vector3 *pos = &ent->client->ps.origin;
	#elif defined(PROJECT_CGAME)
		const vector3 *pos = ((int)(ent - ents) == cg.clientNum)
			? &cg.predictedPlayerState.origin
			: &ent->currentState.pos.trBase; // not cent->lerpOrigin?
	#endif

		Vector_CreateRef( L, pos->x, pos->y, pos->z );
		return 1;
	}

	static int Player_GetSaberStyle( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushinteger( L, ent->client->ps.fd.saberDrawAnimLevel );
	#elif defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
			lua_pushinteger( L, cg.predictedPlayerState.fd.saberDrawAnimLevel );
		}
		else {
			lua_pushnil( L );
		}
	#endif

		return 1;
	}

	static int Player_GetScore( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushinteger( L, ent->client->ps.persistant[PERS_SCORE] );
	#elif defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
			lua_pushinteger( L, cg.predictedPlayerState.persistant[PERS_SCORE] );
		}
		else {
			lua_pushinteger( L, cg.scores[cgs.clientinfo[(int)(ent - ents)].score].score );
		}
	#endif
		return 1;
	}

	static int Player_GetTeam( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushinteger( L, ent->client->sess.sessionTeam );
	#elif defined(PROJECT_CGAME)
		if ( (int)(ent - ents) == cg.clientNum ) {
			lua_pushinteger( L, cg.predictedPlayerState.persistant[PERS_TEAM] );
		}
		else {
			lua_pushinteger( L, cgs.clientinfo[(int)(ent - ents)].team );
		}
	#endif
		return 1;
	}

	static int Player_GetTorsoAnim( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );
	#if defined(PROJECT_CGAME)
		entityState_t *es = &cg_entities[(int)(ent - ents)].currentState;
	#endif

	#if defined(PROJECT_CGAME)
		if ( ps ) {
	#endif
			lua_pushinteger( L, ps->torsoAnim );
	#if defined(PROJECT_CGAME)
		}
		else {
			lua_pushinteger( L, es->torsoAnim );
		}
	#endif
		return 1;
	}

	static int Player_GetVelocity( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		const vector3 *vel = &ent->client->ps.velocity;
	#elif defined(PROJECT_CGAME)
		const vector3 *vel = ((int)(ent - ents) == cg.clientNum)
			? &cg.predictedPlayerState.velocity
			: &ent->currentState.pos.trDelta;
	#endif

		Vector_CreateRef( L, vel->x, vel->y, vel->z );
		return 1;
	}

	static int Player_GetWeapon( lua_State *L, jpluaEntity_t *ent ) {
	#if defined(PROJECT_GAME)
		lua_pushinteger( L, ent->client->ps.weapon );
	#elif defined(PROJECT_CGAME)
		lua_pushinteger( L, ent->currentState.weapon );
	#endif
		return 1;
	}

	static int Player_GetJetpackFuel( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );
		if ( ps ) {
			lua_pushinteger( L, ps->jetpackFuel );
		}
		else {
			lua_pushnil( L );
		}

		return 1;
	}

	static int Player_GetCloakFuel( lua_State *L, jpluaEntity_t *ent ) {
		const playerState_t *ps = GetPlayerstate( ent );
		if ( ps ) {
			lua_pushinteger( L, ps->cloakFuel );
		}
		else {
			lua_pushnil( L );
		}

		return 1;
	}

	static int Player_GetCloaked( lua_State *L, jpluaEntity_t *ent ){
		const playerState_t *ps = GetPlayerstate( ent );
		if ( ps ) {
			lua_pushboolean( L, ps->powerups[PW_CLOAKED] ? 1 : 0 );
		}
	#if defined(PROJECT_CGAME)
		else {
			const entityState_t *es = &ents[(int)(ent - ents)].currentState;
			lua_pushboolean( L, (es->powerups & (1 << PW_CLOAKED)) ? 1 : 0 );
		}
	#endif

		return 1;
	}

	#if defined(PROJECT_GAME)
	static void Player_SetAdminPrivs( lua_State *L, jpluaEntity_t *ent ) {
		if ( ent->client->pers.adminUser ) {
			uint32_t old = ent->client->pers.adminUser->privileges;
			trap->Print( S_COLOR_GREEN "JPLua " S_COLOR_YELLOW "Warning you've changed privileges of logged in player "
				"[OLD: %u]", old
			);
			ent->client->pers.adminUser->privileges = luaL_checkinteger( L, 3 );
		}
		else {
			ent->client->pers.tempprivs = luaL_checkinteger( L, 3 );
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetAngles( lua_State *L, jpluaEntity_t *ent ) {
		const vector3 *vec = CheckVector( L, 3 );
		VectorCopy( vec, &ent->client->ps.viewangles );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetArmor( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.stats[STAT_ARMOR] = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetEFlags( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.eFlags = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetEFlags2( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.eFlags2 = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetFlags( lua_State *L, jpluaEntity_t *ent ) {
		ent->flags = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetForce( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.fd.forcePower = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetHealth( lua_State *L, jpluaEntity_t *ent ) {
		ent->health = ent->client->ps.stats[STAT_HEALTH] = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetEmpowered( lua_State *L, jpluaEntity_t *ent ) {
		bool doEmpower = lua_toboolean( L, 3 ) ? true : false;
		if ( doEmpower ) {
			if ( !ent->client->pers.adminData.empowered ) {
				Empower_On( ent );
			}
		}
		else {
			if ( ent->client->pers.adminData.empowered ) {
				Empower_Off( ent );
			}
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetGhost( lua_State *L, jpluaEntity_t *ent ) {
		bool doGhost = lua_toboolean( L, 3 ) ? true : false;
		if ( doGhost ) {
			if ( !ent->client->pers.adminData.isGhost ) {
				Ghost_On( ent );
			}
		}
		else {
			if ( ent->client->pers.adminData.isGhost ) {
				Ghost_Off( ent );
			}
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetHolstered( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.saberHolstered = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetMerced( lua_State *L, jpluaEntity_t *ent ) {
		bool doMerc = lua_toboolean( L, 3 ) ? true : false;
		if ( doMerc ) {
			if ( !ent->client->pers.adminData.merc ) {
				Merc_On( ent );
			}
		}
		else {
			if ( ent->client->pers.adminData.merc ) {
				Merc_Off( ent );
			}
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetProtected( lua_State *L, jpluaEntity_t *ent ) {
		const bool isProtected = (ent->client->ps.eFlags & EF_INVULNERABLE);
		bool doProtect = lua_toboolean( L, 3 );
		if ( doProtect ) {
			if ( !isProtected ) {
				ent->client->ps.eFlags |= EF_INVULNERABLE;
				ent->client->invulnerableTimer = INT32_MAX;
			}
		}
		else {
			if ( isProtected ) {
				ent->client->ps.eFlags &= ~EF_INVULNERABLE;
				ent->client->invulnerableTimer = level.time;
			}
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetSilenced( lua_State *L, jpluaEntity_t *ent ) {
		bool doSilence = lua_toboolean( L, 3 ) ? true : false;
		if ( doSilence ) {
			if ( !ent->client->pers.adminData.silenced ) {
				ent->client->pers.adminData.silenced = true;
			}
		}
		else {
			if ( ent->client->pers.adminData.silenced ) {
				ent->client->pers.adminData.silenced = false;
			}
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetSlept( lua_State *L, jpluaEntity_t *ent ) {
		bool doSleep = lua_toboolean( L, 3 ) ? true : false;
		if ( doSleep ) {
			if ( !ent->client->pers.adminData.isSlept ) {
				G_SleepClient( ent->client );
			}
		}
		else {
			if ( ent->client->pers.adminData.isSlept ) {
				G_WakeClient( ent->client );
			}
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetLegsAnim( lua_State *L, jpluaEntity_t *ent ) {
		int value = luaL_checkinteger(L, 3);
		if ( value >= FACE_TALK0 && value < MAX_ANIMATIONS ) {
			ent->client->ps.legsAnim = value;
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetName( lua_State *L, jpluaEntity_t *ent ) {
		const char *name = luaL_checkstring(L, 3);
		char info[MAX_INFO_STRING], oldName[MAX_NETNAME];

		if (!name || !*name || strlen(name) >= MAX_NETNAME)
			return;

		Q_strncpyz(oldName, ent->client->pers.netname, sizeof(oldName));

		ClientCleanName(name, ent->client->pers.netname, sizeof(ent->client->pers.netname));

		if (!strcmp(oldName, ent->client->pers.netname))
			return;

		Q_strncpyz(ent->client->pers.netnameClean, ent->client->pers.netname, sizeof(ent->client->pers.netnameClean));
		Q_CleanString(ent->client->pers.netnameClean, STRIP_COLOUR);

		if (CheckDuplicateName(ent->s.number)) {
			Q_strncpyz(ent->client->pers.netnameClean, ent->client->pers.netname, sizeof(ent->client->pers.netnameClean));
			Q_CleanString(ent->client->pers.netnameClean, STRIP_COLOUR);
		}

		// update clientinfo
		trap->GetConfigstring(CS_PLAYERS + ent->s.number, info, sizeof(info));
		Info_SetValueForKey(info, "n", name);
		trap->SetConfigstring(CS_PLAYERS + ent->s.number, info);

		// update userinfo (in engine)
		trap->GetUserinfo(ent->s.number, info, sizeof(info));
		Info_SetValueForKey(info, "name", name);
		trap->SetUserinfo(ent->s.number, info);
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetPosition( lua_State *L, jpluaEntity_t *ent ) {
		vector3 *origin = CheckVector(L, 3);
		VectorCopy(origin, &ent->client->ps.origin);
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetSaberStyle( lua_State *L, jpluaEntity_t *ent ) {
		int value = luaL_checkinteger(L, 3);
		if ( value > SS_NONE && value < SS_NUM_SABER_STYLES ) {
			ent->client->ps.fd.saberDrawAnimLevel = value;
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetScore( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.persistant[PERS_SCORE] = luaL_checkinteger(L, 3);
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetTeam( lua_State *L, jpluaEntity_t *ent ) {
		SetTeam(ent, luaL_checkstring(L, 3), qtrue);
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetTorsoAnim( lua_State *L, jpluaEntity_t *ent ) {
		int value = luaL_checkinteger(L, 3);
		if ( value >= FACE_TALK0 && value < MAX_ANIMATIONS ) {
			ent->client->ps.torsoAnim = value;
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetVelocity( lua_State *L, jpluaEntity_t *ent ) {
		VectorCopy(CheckVector(L, 3), &ent->client->ps.velocity);
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetWeapon( lua_State *L, jpluaEntity_t *ent ) {
		int value = luaL_checkinteger(L, 3);
		if ( value > WP_NONE && value < WP_NUM_WEAPONS ) {
			ent->client->ps.weapon = value;
		}
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetJetpackFuel( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.jetpackFuel = luaL_checkinteger( L, 3 );
	}
	#endif

	#if defined(PROJECT_GAME)
	static void Player_SetCloakFuel( lua_State *L, jpluaEntity_t *ent ) {
		ent->client->ps.cloakFuel = luaL_checkinteger( L, 3 );
	}
	#endif

	static int Player_ToEntity( lua_State *L, jpluaEntity_t *ent ) {
		Entity_CreateRef( L, ent );
		return 1;
	}

	#if defined(PROJECT_GAME)
	static void Player_SetCloaked( lua_State *L, jpluaEntity_t *ent ) {
		bool doCloak = lua_toboolean( L, 3 ) ? true : false;
		if ( doCloak ) {
			if ( ent->client->ps.powerups[PW_CLOAKED] ) {
				Jedi_Cloak( ent );
			}
		}
		else {
			if ( ent->client->ps.powerups[PW_CLOAKED] ) {
				Jedi_Decloak( ent );
			}
		}
	}
	#endif

	static const property_t playerProperties [] = {
	#if defined(PROJECT_GAME)
		{
			"adminPrivileges",
			Player_GetAdminPrivs,
			Player_SetAdminPrivs
		},
	#endif
		{
			"ammo",
			Player_GetAmmo,
			nullptr
		},
		{
			"angles",
			Player_GetAngles,
	#if defined(PROJECT_GAME)
			Player_SetAngles
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"armor",
			Player_GetArmor,
	#if defined(PROJECT_GAME)
			Player_SetArmor
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"cloakfuel",
			Player_GetCloakFuel,
	#if defined(PROJECT_GAME)
			Player_SetCloakFuel
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"duelPartner",
			Player_GetDuelingPartner,
			nullptr
		},
		{
			"eFlags",
			Player_GetEFlags,
	#if defined(PROJECT_GAME)
			Player_SetEFlags
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"eFlags2",
			Player_GetEFlags2,
	#if defined(PROJECT_GAME)
			Player_SetEFlags2
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"entity",
			Player_ToEntity,
			nullptr
		},
	#if defined(PROJECT_GAME)
		//TODO: move to entity object
		{
			"flags",
			Player_GetFlags,
			Player_SetFlags
		},
	#endif
		{
			"force",
			Player_GetForce,
	#if defined(PROJECT_GAME)
			Player_SetForce
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"health",
			Player_GetHealth,
	#if defined(PROJECT_GAME)
			Player_SetHealth
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"id",
			Player_GetID,
			NULL
		},
	#if defined(PROJECT_GAME)
		{
			"isAdmin",
			Player_GetAdmin,
			NULL
		},
	#endif
		{
			"isAlive",
			Player_GetAlive,
			NULL
		},
		{
			"isBot",
			Player_GetBot,
			NULL
		},
		{
			"isCloaked",
			Player_GetCloaked,
	#if defined(PROJECT_GAME)
			Player_SetCloaked
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"isDueling",
			Player_GetDueling,
			NULL
		},
	#if defined(PROJECT_GAME)
		{
			"isEmpowered",
			Player_GetEmpowered,
			Player_SetEmpowered
		},
	#endif
	#if defined(PROJECT_GAME)
		{
			"isGhosted",
			Player_GetGhost,
			Player_SetGhost
		},
	#endif
		{
			"isHolstered",
			Player_GetHolstered,
	#if defined(PROJECT_GAME)
			Player_SetHolstered
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"isInAir",
			Player_GetInAir,
			nullptr
		},
	#if defined(PROJECT_GAME)
		{
			"isMerc",
			Player_GetMerced,
			Player_SetMerced
		},
	#endif
		{
			"isProtected",
			Player_GetProtected,
	#if defined(PROJECT_GAME)
			Player_SetProtected
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
	#if defined(PROJECT_GAME)
		{
			"isSilenced",
			Player_GetSilenced,
			Player_SetSilenced
		},
	#endif
	#if defined(PROJECT_GAME)
		{
			"isSlept",
			Player_GetSlept,
			Player_SetSlept
		},
	#endif
		{
			"isUnderwater",
			Player_GetUnderwater,
			NULL
		},
		{   "jetpackfuel",
			Player_GetJetpackFuel,
	#if defined(PROJECT_GAME)
			Player_SetJetpackFuel
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"lastPickup",
	#if defined(PROJECT_GAME)
			nullptr,
	#elif defined(PROJECT_CGAME)
			Player_GetLastPickup,
	#endif
			NULL
		},
		{
			"legsAnim",
			Player_GetLegsAnim,
	#if defined(PROJECT_GAME)
			Player_SetLegsAnim
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"location",
			Player_GetLocation,
			NULL
		},
		{
			"maxAmmo",
			Player_GetMaxAmmo,
			NULL
		},
		{
			"name",
			Player_GetName,
	#if defined(PROJECT_GAME)
			Player_SetName
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"position",
			Player_GetPosition,
	#if defined(PROJECT_GAME)
			Player_SetPosition
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"saberStyle",
			Player_GetSaberStyle,
	#if defined(PROJECT_GAME)
			Player_SetSaberStyle
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"score",
			Player_GetScore,
	#if defined(PROJECT_GAME)
			Player_SetScore
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"team",
			Player_GetTeam,
	#if defined(PROJECT_GAME)
			Player_SetTeam
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"torsoAnim",
			Player_GetTorsoAnim,
	#if defined(PROJECT_GAME)
			Player_SetTorsoAnim
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"velocity",
			Player_GetVelocity,
	#if defined(PROJECT_GAME)
			Player_SetVelocity
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
		{
			"weapon",
			Player_GetWeapon,
	#if defined(PROJECT_GAME)
			Player_SetWeapon
	#elif defined(PROJECT_CGAME)
			nullptr
	#endif
		},
	};
	static const size_t numPlayerProperties = ARRAY_LEN( playerProperties );

	static int Player_Index( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		const char *key = lua_tostring( L, 2 );
		int returnValues = 0;

		lua_getmetatable( L, 1 );
		lua_getfield( L, -1, key );
		if ( !lua_isnil( L, -1 ) ) {
			return 1;
		}

		// assume it's a field
		const property_t *property = (property_t *)bsearch( key, playerProperties, numPlayerProperties,
			sizeof(property_t), propertycmp
		);
		if ( property ) {
			if ( property->Get ) {
				returnValues += property->Get( L, ents + player->clientNum );
			}
		}
		else {
			lua_pushnil( L );
			returnValues++;
		}

		return returnValues;
	}

	static int Player_NewIndex( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		const char *key = lua_tostring( L, 2 );

		lua_getmetatable( L, 1 );
		lua_getfield( L, -1, key );

		if ( !lua_isnil( L, -1 ) ) {
			return 1;
		}

		// assume it's a field
		const property_t *property = (property_t *)bsearch(key, playerProperties, numPlayerProperties,
			sizeof(property_t), propertycmp
		);
		if ( property ) {
			if ( property->Set ) {
				property->Set( L, ents + player->clientNum );
			}
		}
		else {
		}

		return 0;
	}

	//Func: GetPlayer( clientNum )
	//Retn: Player object
	int GetPlayer( lua_State *L ) {
	#if defined(PROJECT_GAME)
		int clientNum;

		if ( lua_type( L, 1 ) == LUA_TNUMBER ) {
			clientNum = lua_tointeger( L, 1 );
		}

		else if ( lua_type( L, 1 ) == LUA_TSTRING ) {
			const char *name = lua_tostring( L, 1 );
			clientNum = G_ClientFromString( NULL, name, FINDCL_SUBSTR );
			if ( clientNum == -1 ) {
				lua_pushnil( L );
				return 1;
			}
		}

		else {
			lua_pushnil( L );
			return 1;
		}

		Player_CreateRef( L, clientNum );
		return 1;
	#elif defined(PROJECT_CGAME)
		int num = -1;
		uint32_t clientsFound = 0;

		if ( lua_type( L, 1 ) == LUA_TNUMBER ) {
			num = lua_tointeger( L, 1 );
		}

		else if ( lua_type( L, 1 ) == LUA_TSTRING ) {
			const char *name = lua_tostring( L, 1 );
			int numFound = 0;
			//RAZTODO: copy G_ClientFromString
			for ( int i = 0; i < cgs.maxclients; i++ ) {
				char nameClean[MAX_NETNAME], nameClean2[MAX_NETNAME];

				if ( !cgs.clientinfo[i].infoValid ) {
					continue;
				}

				Q_strncpyz( nameClean, cgs.clientinfo[i].name, sizeof(nameClean) );
				Q_strncpyz( nameClean2, name, sizeof(nameClean2) );
				Q_CleanString( nameClean, STRIP_COLOUR );
				Q_CleanString( nameClean2, STRIP_COLOUR );
				if ( !Q_stricmp( nameClean, nameClean2 ) ) {
					num = i;
					clientsFound |= (1 << i);
					numFound++;
				}
			}

			if ( numFound > 1 ) {
				int top = 0;
				lua_pushnil( L );
				lua_pushstring( L, "Multiple matches" );

				lua_newtable( L );
				top = lua_gettop( L );
				for ( int i = 0; i < cgs.maxclients; i++ ) {
					if ( clientsFound & (1 << i) ) {
						lua_pushnumber( L, i );
						Player_CreateRef( L, i );
						lua_settable( L, top );
					}
				}
				return 3;
			}
			else if ( !numFound ) {
				lua_pushnil( L );
				return 1;
			}
		}

		else {//if ( lua_type( L, 1 ) == LUA_TNIL )
			num = cg.clientNum;
		}

		Player_CreateRef( L, num );
		return 1;
	#endif
	}

	//Func: Player1 == Player2
	//Retn: boolean value of whether Player1 is the same client as Player2
	static int Player_Equals( lua_State *L ) {
		luaPlayer_t *p1 = CheckPlayer( L, 1 );
		luaPlayer_t *p2 = CheckPlayer( L, 2 );
		lua_pushboolean( L, (p1->clientNum == p2->clientNum) ? 1 : 0 );
		return 1;
	}

	//Func: tostring( Player )
	//Retn: string representing the Player instance (for debug/error messages)
	static int Player_ToString( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		lua_pushfstring( L, "Player(%d)", player->clientNum );
		return 1;
	}

	#if defined(PROJECT_CGAME)
	static int Player_GetClientInfo( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		clientInfo_t *ci = &cgs.clientinfo[player->clientNum];
		entityState_t *es = &cg_entities[player->clientNum].currentState;

		score_t	*score = NULL; // for ping and other scoreboard information
		for ( int i = 0; i < cg.numScores; i++ ) {
			if ( cg.scores[i].client == player->clientNum ) {
				score = &cg.scores[i];
				break;
			}
		}

		lua_newtable( L );
		int top = lua_gettop( L );

		lua_pushstring( L, "saberName" ); lua_pushstring( L, ci->saberName ); lua_settable( L, top );
		lua_pushstring( L, "saber2Name" ); lua_pushstring( L, ci->saber2Name ); lua_settable( L, top );
		lua_pushstring( L, "name" ); lua_pushstring( L, ci->name ); lua_settable( L, top );
		lua_pushstring( L, "team" ); lua_pushinteger( L, ci->team ); lua_settable( L, top );
		lua_pushstring( L, "duelTeam" ); lua_pushinteger( L, ci->duelTeam ); lua_settable( L, top );
		lua_pushstring( L, "botSkill" ); lua_pushinteger( L, ci->botSkill ); lua_settable( L, top );
		lua_pushstring( L, "icolor1" ); lua_pushinteger( L, ci->icolor1 ); lua_settable( L, top );
		lua_pushstring( L, "icolor2" ); lua_pushinteger( L, ci->icolor2 ); lua_settable( L, top );
		lua_pushstring( L, "score" ); lua_pushinteger( L, ci->score ); lua_settable( L, top );
		lua_pushstring( L, "modelName" ); lua_pushstring( L, ci->modelName ); lua_settable( L, top );
		lua_pushstring( L, "skinName" ); lua_pushstring( L, ci->skinName ); lua_settable( L, top );
		lua_pushstring( L, "deferred" ); lua_pushboolean( L, !!ci->deferred ); lua_settable( L, top );
		lua_pushstring( L, "gender" ); lua_pushinteger( L, ci->gender ); lua_settable( L, top );
		if ( score ) {
			lua_pushstring( L, "ping" ); lua_pushinteger( L, (player->clientNum == cg.clientNum) ? cg.snap->ping : score->ping ); lua_settable( L, top );
			lua_pushstring( L, "time" ); lua_pushinteger( L, score->time ); lua_settable( L, top );
			lua_pushstring( L, "deaths" ); lua_pushinteger( L, score->deaths ); lua_settable( L, top );
		}
		else {
			lua_pushstring( L, "ping" );
			if ( player->clientNum == cg.clientNum )
				lua_pushinteger( L, cg.snap->ping );
			else
				lua_pushnil( L );
			lua_settable( L, top );
			lua_pushstring( L, "time" ); lua_pushnil( L ); lua_settable( L, top );
			lua_pushstring( L, "deaths" ); lua_pushnil( L ); lua_settable( L, top );
		}

		lua_pushstring( L, "rgb1" ); Vector_CreateRef( L, ci->rgb1.r, ci->rgb1.g, ci->rgb1.b ); lua_settable( L, top );
		lua_pushstring( L, "rgb2" ); Vector_CreateRef( L, ci->rgb2.r, ci->rgb2.g, ci->rgb2.b ); lua_settable( L, top );
		lua_pushstring( L, "skinRGB" ); Vector_CreateRef( L, es->customRGBA[0], es->customRGBA[1], es->customRGBA[2] ); lua_settable( L, top );

		return 1;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_GetAdminData( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		gentity_t *ent = &g_entities[player->clientNum];

		lua_newtable( L );
		int top = lua_gettop( L );
		lua_pushstring( L, "login" ); lua_pushstring( L, ent->client->pers.adminUser->user ); lua_settable( L, top );
		lua_pushstring( L, "password" ); lua_pushstring(L, ent->client->pers.adminUser->password ); lua_settable( L, top );
		lua_pushstring( L, "privileges" ); lua_pushinteger(L, ent->client->pers.adminUser->privileges ); lua_settable( L, top );
		lua_pushstring( L, "loginmsg" ); lua_pushstring(L, ent->client->pers.adminUser->loginMsg ); lua_settable( L, top );
		lua_pushstring( L, "rank" ); lua_pushinteger(L, ent->client->pers.adminUser->rank ); lua_settable( L, top );
		lua_pushstring( L, "logineffect" ); lua_pushinteger(L, ent->client->pers.adminUser->logineffect ); lua_settable( L, top );

		return 1;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_GetUserinfo( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		char userinfo[MAX_INFO_STRING];
		const char *info;
		const char *name = lua_tostring(L, 2);
		if (lua_isnil(L, 2) || !Q_stricmp(name, "") || !Q_stricmp(name, "-1") || !name ) {
			trap->GetUserinfo(player->clientNum, userinfo, sizeof(userinfo));
			PushInfostring(L, userinfo);
			return 1;
		}
		trap->GetUserinfo( player->clientNum, userinfo, sizeof(userinfo) );
		info = Info_ValueForKey(userinfo, name);

		lua_pushstring(L, info);
		return 1;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_Give( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		gentity_t *ent = &g_entities[player->clientNum];
		const char *type = luaL_checkstring( L, 2 );
		int id = luaL_checkinteger( L, 3 );
		int value = lua_tointeger( L, 4 );

		if ( !Q_stricmp( type, "weapon" ) ) {
			if ( id == -1 ) {
				ent->client->ps.stats[STAT_WEAPONS] = ((1 << LAST_USEABLE_WEAPON) - 1) & ~1;
			}
			if ( id <= WP_NONE || id >= WP_NUM_WEAPONS ) {
				return 0;
			}

			ent->client->ps.stats[STAT_WEAPONS] |= (1 << id);
		}
		else if ( !Q_stricmp( type, "powerup" ) ) {
			if ( id <= PW_NONE || id >= PW_NUM_POWERUPS ) {
				return 0;
			}

			ent->client->ps.powerups[id] = level.time + value;
		}
		else if ( !Q_stricmp( type, "item" ) ) {
			if ( id == -1 ) {
				for ( int i = 0; i < HI_NUM_HOLDABLE; i++ ) {
					ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
				}
			}
			if ( id <= HI_NONE || id >= HI_NUM_HOLDABLE ) {
				return 0;
			}

			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << id);
		}
		else if ( !Q_stricmp( type, "ammo" ) ) {
			if ( id == -1 ) {
				for ( int i = 0; i < AMMO_MAX; i++ ) {
					ent->client->ps.ammo[i] = ammoMax[i];
				}
			}
			if ( id <= 0 || id >= AMMO_MAX ) {
				return 0;
			}

			ent->client->ps.ammo[id] = value;
		}

		return 0;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_Kick( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		const char *reason = lua_tostring( L, 2 );

		trap->DropClient( player->clientNum, reason ? reason : "was kicked" );

		return 0;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_Kill( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		gentity_t *ent = &g_entities[player->clientNum];

		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		player_die( ent, ent, ent, 100000, MOD_SUICIDE );

		return 0;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_Slap( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer(L, 1);
		gentity_t *ent = &g_entities[player->clientNum];

		Slap(ent);

		return 0;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_Take( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		gentity_t *ent = &g_entities[player->clientNum];
		const char *type = luaL_checkstring( L, 2 );
		int id = luaL_checkinteger( L, 3 );
		int i, newWeapon = -1, selectedWeapon = ent->client->ps.weapon;

		if ( !Q_stricmp( type, "weapon" ) ) {
			if ( id <= WP_NONE || id >= WP_NUM_WEAPONS ) {
				return 0;
			}

			ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << id);

			for ( i = WP_SABER; i < WP_NUM_WEAPONS; i++ ) {
				if ( ent->client->ps.stats[STAT_WEAPONS] & (1 << i) ) {
					newWeapon = i;
					break;
				}
			}

			if ( newWeapon == WP_NUM_WEAPONS ) {
				for ( i = WP_STUN_BATON; i < WP_SABER; i++ ) {
					if ( ent->client->ps.stats[STAT_WEAPONS] & (1 << i) ) {
						newWeapon = i;
						break;
					}
				}
				if ( newWeapon == WP_SABER ) {
					newWeapon = WP_NONE;
				}
			}

			ent->client->ps.weapon = (newWeapon == -1) ? 0 : newWeapon;

			G_AddEvent( ent, EV_NOAMMO, selectedWeapon );
		}

		else if ( !Q_stricmp( type, "powerup" ) ) {
			if ( id <= PW_NONE || id >= PW_NUM_POWERUPS ) {
				return 0;
			}

			ent->client->ps.powerups[id] = 0;
		}
		else if ( Q_stricmp( type, "item" ) ) {
			if ( id <= HI_NONE || id >= HI_NUM_HOLDABLE ) {
				return 0;
			}
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << id);
		}

		return 0;
	}
	#endif

	#if defined(PROJECT_GAME)
	static int Player_Teleport( lua_State *L ) {
		luaPlayer_t *player = CheckPlayer( L, 1 );
		gentity_t *ent = &g_entities[player->clientNum];
		vector3 *pos = CheckVector( L, 2 );
		vector3 *angles = CheckVector( L, 3 );

		TeleportPlayer( ent, pos, angles );

		return 0;
	}

	static int Player_SetAnim(lua_State *L){
		gentity_t *ent = &g_entities[CheckPlayer( L, 1 )->clientNum];

		int animLoop = luaL_checkinteger(L,2), animLeave = luaL_checkinteger(L,3), flags = luaL_checkinteger(L,4);
		emote_t emote;
		emote.name = "jplua emote";
		emote.animLoop = (animNumber_t)animLoop;
		emote.animLeave = (animNumber_t)animLeave;
		emote.flags = flags;

		SetEmote(ent, &emote);
		return 0;
	}
	#endif

	static int Player_ToEntity(lua_State *L){
		luaPlayer_t *player = CheckPlayer(L, 1);
		jpluaEntity_t *ent = &ents[player->clientNum];
		Entity_CreateRef(L, ent);
		return 1;
	}

	static const struct luaL_Reg jplua_player_meta[] = {
		{ "__index", Player_Index},
		{ "__newindex", Player_NewIndex},
		{ "__eq", Player_Equals },
		{ "__tostring", Player_ToString },
	#if defined(PROJECT_CGAME)
		{ "GetClientInfo", Player_GetClientInfo },
	#elif defined(PROJECT_GAME)
		{ "GetAdminData", Player_GetAdminData},
		{ "GetUserinfo", Player_GetUserinfo },
		{ "Give", Player_Give },
		{ "Kick", Player_Kick },
		{ "Kill", Player_Kill },
		{ "Slap", Player_Slap},
		{ "Take", Player_Take },
		{ "Teleport", Player_Teleport },
		{ "SetAnim", Player_SetAnim },
	#endif
		{ "ToEntity", Player_ToEntity },
		{ NULL, NULL }
	};

	// Register the Player class for Lua
	void Register_Player( lua_State *L ) {
		const luaL_Reg *r;

		luaL_newmetatable( L, PLAYER_META ); // Create metatable for Player class, push on stack

		// Lua won't attempt to directly index userdata, only via metatables
		// Make this metatable's __index loopback to itself so can index the object directly
		lua_pushstring( L, "__index" );
		lua_pushvalue( L, -2 ); // Re-push metatable to top of stack
		lua_settable( L, -3 ); // metatable.__index = metatable

		// fill metatable with fields
		for ( r = jplua_player_meta; r->name; r++ ) {
			lua_pushcfunction( L, r->func );
			lua_setfield( L, -2, r->name );
		}

		lua_pop( L, -1 ); // Pop the Player class metatable from the stack
	}

} // namespace JPLua

#endif // JPLUA
