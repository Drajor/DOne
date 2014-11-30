#include "ZoneData.h"
#include "DataStore.h"
#include "Utility.h"
#include "LogSystem.h"
/*
2243 South Qeynos
2244 North Qeynos
2245 Surefall Glade
2246 Qeynos Hills
2247 Highpass Hold
2248 HighKeep
2249 North Freeport
2250 West Freeport
2251 East Freeport
2252 Clan RunnyEye
2253 West Karana
2254 North Karana
2255 South Karana
2256 East Karana
2257 Gorge of King Xorbb
2258 BlackBurrow
2259 Infected Paw
2260 Rivervale
2261 Kithicor Forest
2262 West Commonlands
2263 East Commonlands
2264 Erudin Palace
2265 Erudin
2266 Nektulos Forest
2267 Sunset Home
2268 Lavastorm Mountains
2269 Nektropos
2270 Halas
2271 Everfrost Peaks
2272 Solusek's Eye
2273 Nagafen's Lair
2274 Misty Thicket
2275 North Ro
2276 South Ro
2277 Befallen
2278 Oasis of Marr
2279 Toxxulia Forest
2280 The Ruins of Old Paineel
2281 Neriak Foreign Quarter
2282 Neriak Commons
2283 Neriak Third Gate
2284 Neriak Palace
2285 Najena
2286 Qeynos Catacombs
2287 Innothule Swamp
2288 The Feerrott
2289 Cazic-Thule
2290 Oggok
2291 Mountains of Rathe
2292 Lake Rathetear
2293 Grobb
2294 Aviak Village
2295 Greater Faydark
2296 Ak'Anon
2297 Steamfont Mountains
2298 Lesser Faydark
2299 Clan Crushbone
2300 Castle Mistmoore
2301 Loading
2302 Bad syntax in grammar rule %1
2303 Felwithe
2304 Estate of Unrest
2305 Kedge Keep
2306 Upper Guk
2307 Lower Guk
2308 Kaladim
2309 Butcherblock Mountains
2310 Ocean of Tears
2311 Dagnor's Cauldron
2312 Plane of Sky
2313 Plane of Fear
2314 Permafrost Keep
2315 Kerra Isle
2316 Paineel
2317 The Plane of Hate
2318 Entry Deleted due to buffer overrun
2319 The Field of Bone
2320 Warsliks Wood
2321 Temple of Solusek Ro
2322 Temple of Droga
2323 West Cabilis
2324 Swamp of No Hope
2325 Firiona Vie
2326 Lake of Ill Omen
2327 Dreadlands
2328 Burning Woods
2329 Kaesora
2330 Old Sebilis
2331 City of Mist
2332 Skyfire Mountains
2333 Frontier Mountains
2334 The Overthere
2335 The Emerald Jungle
2336 Trakanon's Teeth
2337 Timorous Deep
2338 Kurn's Tower
2339 Erud's Crossing
2340 Stonebrunt Mountains
2341 The Warrens
2342 Karnor's Castle
2343 Chardok
2344 Dalnir
2345 Howling Stones
2346 East Cabilis
2347 Mines of Nurga
2348 Veeshan's Peak
2349 Veksar
2350 Iceclad Ocean
2351 Tower of Frozen Shadow
2352 Velketor's Labyrinth
2353 Kael Drakkal
2354 Skyshrine
2355 Thurgadin
2356 Eastern Wastes
2357 Cobalt Scar
2358 Great Divide
2359 The Wakening Land
2360 Western Wastes
2361 Crystal Caverns
2362 Dragon Necropolis
2363 Temple of Veeshan
2364 Siren's Grotto
2365 Plane of Mischief
2366 Plane of Growth
2367 Sleeper's Tomb
2368 Icewell Keep
2369 Marauder's Mire
2370 Shadow Haven
2371 The Bazaar
2372 The Nexus
2373 Echo Caverns
2374 Acrylia Caverns
2375 Shar Vahl
2376 Paludal Caverns
2377 Fungus Grove
2378 Vex Thal
2379 Sanctus Seru
2380 Katta Castellum
2381 Netherbian Lair
2382 Ssraeshza Temple
2383 Grieg's End
2384 The Deep
2385 Shadeweaver's Thicket
2386 Hollowshade Moor
2387 Grimling Forest
2388 Marus Seru
2389 Mons Letalis
2390 The Twilight Sea
2391 The Grey
2392 The Tenebrous Mountains
2393 The Maiden's Eye
2394 Dawnshroud Peaks
2395 The Scarlet Desert
2396 The Umbral Plains
2397 Akheva Ruins
2398 The Jaggedpine Forest
2399 Tutorial Zone
2400 EverQuest Tutorial
2401 The Arena
*/
/*
3385 Abysmal Sea
3386 Natimbi, The Broken Shores
3387 Qinimi, Court of Nihilia
3388 Riwwi, Coliseum of Games
3389 Barindu, Hanging Gardens
3390 Ferubi, Forgotten Temple of Taelosia
3391 Sewers of Nihilia, Pool of Sludge
3392 Sewers of Nihilia, Lair of Trapped Ones
3393 Sewers of Nihilia, Purifying Plant
3394 Sewers of Nihilia, the Crematory
3395 Tipt, Treacherous Crags
3396 Vxed, The Crumbling Caverns
3397 Yxtta, Pulpit of Exiles
3398 Uqua, The Ocean God Chantry
3399 Kod'Taz, Broken Trial Grounds
*/

/*
3630 Arcstone
3631 Relic
3632 Skylance
3633 The Devastation
3634 The Seething Wall
3635 Sverag, Stronghold of Rage
3636 Razorthorn, Tower of Sullon Zek
3637 Ruins of Takish-Hiz
3638 The Root of Ro
3639 The Elddar Forest
3640 Tunare's Shrine
3641 Theater of Blood
3642 Deathknell, Tower of Dissonance
3643 East Freeport
3644 West Freeport
3645 Freeport Sewers
3646 Academy of Arcane Sciences
3647 Temple of Mith Marr
3648 Freeport Militia House
3649 Arena
3650 City Hall
3651 Theater
3652 Hall of Truth

3655 The Desert of Ro
*/
/*
3675 Crescent Reach
3676 Blightfire Moors
3677 Stone Hive
3678 Goru`kar Mesa
3679 Blackfeather Roost
3680 The Steppes
3681 Icefall Glacier
3682 Valdeholm
3683 Frostcrypt, Throne of the Shade King
3684 Sunderock Springs
3685 Vergalid Mines
3686 Direwind Cliffs
3687 Ashengate, Reliquary of the Scale
*/

/*
4046 Torgiran
4047 Nadox
4048 Gunthak
4049 Dulak's Harbor
4050 Hatesfury
*/

ZoneDataManager::~ZoneDataManager(){
	
}

const bool ZoneDataManager::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	Log::status("[Zone Data] Initialising.");
	EXPECTED_BOOL(DataStore::getInstance().loadZoneData(mZoneData));
	Log::info("[Zone Data] Loaded data for " + std::to_string(mZoneData.size()) + " Zones.");
	
	mInitialised = true;
	return true;
}

ZoneData* ZoneDataManager::_find(const uint16 pZoneID) const {
	for (auto i : mZoneData) {
		if (pZoneID == i->mID)
			return i;
	}

	return nullptr;
}

const bool ZoneDataManager::getLongName(const uint16 pZoneID, String& pLongName) {
	ZoneData* zoneData = _find(pZoneID);
	EXPECTED_BOOL(zoneData);
	pLongName = zoneData->mLongName;
	return true;
}

const bool ZoneDataManager::getShortName(const uint16 pZoneID, String& pShortName){
	ZoneData* zoneData = _find(pZoneID);
	EXPECTED_BOOL(zoneData);
	pShortName = zoneData->mShortName;
	return true;
}

const bool ZoneDataManager::getLongNameStringID(const uint16 pZoneID, uint32& pStringID) {
	ZoneData* zoneData = _find(pZoneID);
	EXPECTED_BOOL(zoneData);
	pStringID = zoneData->mLongNameStringID;
	return true;
}

ZoneDataSearchResults ZoneDataManager::searchByName(String pSearchText) {
	ZoneDataSearchResults results;
	for (auto i : mZoneData) {
		if (Utility::findCI(i->mShortName, pSearchText) || Utility::findCI(i->mLongName, pSearchText)) {
			results.push_back({i->mID, i->mShortName, i->mLongName});
		}
	}

	return results;
}

const uint16 ZoneDataManager::findFirstByName(const String& pSearchText) {
	for (auto i : mZoneData) {
		if (Utility::compareCI(i->mShortName, pSearchText) || Utility::compareCI(i->mLongName, pSearchText)) {
			return i->mID;
		}
	}

	return 0;
}


const bool ZoneDataManager::getSpawnPoints(const uint16 pZoneID, std::list<SpawnPointData*>& pSpawnPointData) {
	ZoneData* zoneData = _find(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSpawnPointData = zoneData->mSpawnPoints;
	return true;
}

const bool ZoneDataManager::getZonePoints(const uint16 pZoneID, std::list<ZonePointData*>** pZonePoints) {
	ZoneData* zoneData = _find(pZoneID);
	EXPECTED_BOOL(zoneData);
	*pZonePoints = &zoneData->mZonePoints;
	return true;
}

const bool ZoneDataManager::getSafePoint(const uint16 pZoneID, Vector3& pSafePoint) {
	ZoneData* zoneData = _find(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSafePoint.x = zoneData->mSafeX;
	pSafePoint.y = zoneData->mSafeY;
	pSafePoint.z = zoneData->mSafeZ;

	return true;
}
