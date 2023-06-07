#pragma once

#include <vector>

inline std::vector<const char*> autowall_type =
{
	"Disable",
	"Performance",
	"Ultimate"
};

inline std::vector<const char*> autoscope_type =
{
	"Disable",
	"Always",
	"Dynamic"
};

inline std::vector<const char*> antiaim_type =
{
	"Rage",
	"Legit"
};

inline std::vector<const char*> movement_type =
{
	"Stand",
	//"Slow walk",
	"Move",
	"Air",
	//"InCrouch"
};

inline std::vector<const char*> selection =
{
	"Cycle",
	"Near crosshair",
	"Lowest distance",
	"Lowest health",
	"Highest damage"
};

inline std::vector<const char*> scan_points =
{
	"Default",
	"Static",
	"Multi"
};

inline std::vector<const char*> autostop_modifiers =
{
	"Between shots",
	"On lethal",
	"Visible",
	"Center",
	"Force accuracy",
	"Predictive"
};

inline std::vector<const char*> safe_points_conditions =
{
	"Visible",
	"Prefer",
	"In air",
	"In crouch",
	"On limbs",
	"Low damage"
};

inline std::vector<const char*> body_aim_conditions =
{
	"Double tap",
	"Prefer",
	"In crouch",
	"In air"
};

inline std::vector<const char*> head_aim_conditions =
{
	"On shot",
	"On air"
};

inline std::vector<const char*> head_aim_only_conditions =
{
	"On shot",
	"On air"
};

inline std::vector<const char*> hitboxes =
{
	"Head",
	"Upper chest",
	"Chest",
	"Lower chest",
	"Stomach",
	"Pelvis",
	"Arms",
	"Legs",
	"Feets"
};

inline std::vector<const char*> hitboxes_baim =
{
	"Upper chest",
	"Chest",
	"Lower chest",
	"Stomach",
	"Pelvis",
	"Arms",
	"Legs",
	"Feets"
};

inline std::vector<const char*> pitch =
{
	"None",
	"Down",
	"Up",
	"Zero",
	"Random",
	"Jitter",
	"Fake down",
	"Fake up",
	"Fake jitter",
	"Custom"
};

inline std::vector<const char*> yaw =
{
	"Static",
	"Jitter",
	"Spin",
	"Custom"
};

inline std::vector<const char*> baseangle =
{
	"Local view",
	"At targets"
};

inline std::vector<const char*> desync =
{
	"None",
	"Static",
	"Jitter"
};

inline std::vector<const char*> jitter_type =
{
	"SendPacket",
	"Ticks"
};

inline std::vector<const char*> lby_type =
{
	"Normal",
	"Opposite",
	"Sway"
};

inline std::vector<const char*> desync_type =
{
	"Normal",
	"Random"
};

inline std::vector<const char*> body_lean_type =
{
	"Normal",
	"Random"
};

inline std::vector<const char*> proj_combo =
{
	"Icon",
	"Text",
	"Box",
	"Glow"
};

inline std::vector<const char*> weaponplayer =
{
	"Icon",
	"Text"
};

inline std::vector<const char*> weaponesp =
{
	"Icon",
	"Text",
	"Box",
	"Distance",
	"Glow",
	"Ammo"
};

inline std::vector<const char*> hitmarkers =
{
	"Crosshair",
	"World"
};

inline std::vector<const char*> glowtarget =
{
	"Enemy",
	"Teammate",
	"Local"
};

inline std::vector<const char*> glowtype =
{
	"Standard",
	"Pulse",
	"Inner"
};

inline std::vector<const char*> local_chams_type =
{
	"Real",
	"Desync"
};

inline std::vector<const char*> chamsvisact =
{
	"Visible",
	"Invisible"
};

inline std::vector<const char*> chamsvis =
{
	"Visible"
};

inline std::vector<const char*> chamstype =
{
	"Regular",
	"Mettalic",
	"Flat",
	"Pulse",
	"Crystal",
	"Glass",
	"Circuit",
	"Golden",
	"Glow"
};

inline std::vector<const char*> flags =
{
	"Money",
	"Armor",
	"Defuse kit",
	"Scoped",
	"Fakeducking",
	"Vulnerable",
	"Delay",
	"Bomb carrier"
};

inline std::vector<const char*> removals =
{
	"Scope",
	"Zoom",
	"Smoke",
	"Molotov",
	"Flash",
	"Recoil",
	"Landing bob",
	"Postprocessing",
	"Fog",
	"Shadows",
	"3DSky"
};

inline std::vector<const char*> indicators =
{
	"Fake",
	"Desync side",
	"Choke",
	"LC",
	"Anti exploit",
	"Damage override",
	"Safe points",
	"Body aim",
	"Resolver override",
	"Double tap",
	"Hide shots"
};

inline std::vector<const char*> resolver_types =
{
	"Yaw",
	"Pitch"
};

inline std::vector<const char*> resolver_mode =
{
	"Default",
	"Dynamic"
};

inline std::vector<const char*> skybox =
{
	"None",
	"Tibet",
	"Baggage",
	"Italy",
	"Aztec",
	"Vertigo",
	"Daylight",
	"Daylight 2",
	"Clouds",
	"Clouds 2",
	"Gray",
	"Clear",
	"Canals",
	"Cobblestone",
	"Assault",
	"Clouds dark",
	"Night",
	"Night 2",
	"Night flat",
	"Dusty",
	"Rainy",
	"Custom"
};

inline std::vector<const char*> blockbot_type =
{
	"Match Speed",
	"Maximum"
};

inline std::vector<const char*> mainwep =
{
	"None",
	"Auto",
	"AWP",
	"SSG 08"
};

inline std::vector<const char*> secwep =
{
	"None",
	"Dual Berettas",
	"Deagle/Revolver"
};

inline std::vector<const char*> strafes =
{
	"None",
	"Legit",
	"Directional",
	"Rage"
};

inline std::vector<const char*> centersList =
{
	"Off",
	"Australia",
	"Austria",
	"Brazil",
	"Chile",
	"Dubai",
	"France",
	"Germany",
	"Hong Kong",
	"India (Chennai)",
	"India (Mumbai)",
	"Japan",
	"Luxembourg",
	"Netherlands",
	"Peru",
	"Philipines",
	"Poland",
	"Singapore",
	"South Africa",
	"Spain",
	"Sweden",
	"UK",
	"USA (Atlanta)",
	"USA (Seattle)",
	"USA (Chicago)",
	"USA"
};

inline std::vector<const char*> walkbot_type =
{
	"Enable",
	"Cycle walk",
	"Sound alert",
	"Developer mode"
};

inline std::vector<const char*> events_output =
{
	"Console",
	"Chat",
	"Draw"
};

inline std::vector<const char*> events =
{
	"Player hits",
	"Items",
	"Bomb"
};

inline std::vector<const char*> grenades =
{
	"Grenades",
	"Armor",
	"Taser",
	"Defuser"
};

inline std::vector<const char*> fakelags =
{
	"Static",
	"Random",
	"Dynamic",
	"Fluctuate",
	"BreakLC"
};

inline std::vector<const char*> lagstrigger =
{
	"Slow walk",
	"Move",
	"Air",
	"Peek"
};

inline std::vector<const char*> mmregions =
{
	"Warsaw [Poland]",
	"Atlanta [U.S. South]",
	"Mumbai [India]",
	"Guangzhou [China]",
	"Guangzhou #2 [China]",
	"Guangzhou #3 [China]",
	"Guangzhou #4 [China]",
	"Dubai [United Arab Emirates]",
	"Seattle [U.S. North]",
	"Frankfurt [Switzerland]",
	"Sao Paulo [Brazil]",
	"Hong Kong [China]",
	"Washington D.C [U.S. East]",
	"Cape Town [South Africa]",
	"Los Angeles [U.S. West]",
	"London [United Kingdom]",
	"Lima [Peru]",
	"Berlin [Germany]",
	"Chennai [India]",
	"Madrid [Spain]",
	"Manila [Phillipines]",
	"Oklahoma City [Canada]",
	"Chicago [U.S.]",
	"Paris [France]",
	"Guangzhou #5 [China]",
	"Tianjin [China]",
	"Hebei [China]",
	"Wuhan [China]",
	"Jiaxing [China]",
	"Santiago [Chile]",
	"Seoul [South Korea]",
	"Singapore [Australia]",
	"Shanghai [China]",
	"Shanghai #2 [China]",
	"Shanghai #3 [China]",
	"Shanghai #4 [China]",
	"Nakashibetsu [Japan]",
	"Moscow [Russia]",
	"Moscow #2 [Russia]",
	"Sydney [Australia]",
	"Beijing [China]",
	"Beijing #2 [China]",
	"Beijing #3 [China]",
	"Beijing #4 [China]",
	"Tokyo [Japan]",
	"Tokyo #2 [Japan]",
	"Vienna [Austria]",
	"Amsterdam [Netherlands]"
};

inline std::vector<const char*> sounds =
{
	"None",
	"Mettalic",
	"Cod",
	"Bubble",
	"Stapler",
	"Bell",
	"Flick",
	"Custom"
};

inline std::vector<const char*> watermarks =
{
	"None",
	"HackerSense",
	"Default"
};

inline std::vector<const char*> legs =
{
	"Avoid",
	"Slide",
	"Break"
};

inline std::vector<const char*> cooldowns =
{
	"Off",
	"You were kicked from the last match (Competitive cooldown)",
	"You killed too many teammates (Competitive cooldown)",
	"You killed a teammate at round start (Competitive cooldown)",
	"You failed to reconnect to the last match (Competitive cooldown)",
	"You abandoned the last match (Competitive cooldown)",
	"You did too much damage to your teammates (Competitive cooldown)",
	"You did too much damage to your teammates at round start (Competitive cooldown)",
	"This account is permanently untrusted (Global cooldown)",
	"You were kicked from too many recent matches (Competitive cooldown)",
	"Convicted by overwatch - majorly disruptive (Global cooldown)",
	"Convicted by overwatch - minorly disruptive (Global cooldown)",
	"Resolving matchmaking state for your account (Temporary cooldown)",
	"Resolving matchmaking state after the last match (Temporary cooldown)",
	"This account is permanently untrusted (Global cooldown)",
	"(Global cooldown)",
	"You failed to connect by match start. (Competitive cooldown)",
	"You have kicked too many teammates in recent matches (Competitive cooldown)",
	"Congratulations on your recent competitive wins! before you play competitive matches further please wait for matchmaking servers to calibrate your skill group placement based on your lastest performance. (Temporary cooldown)",
	"A server using your game server login token has been banned. your account is now permanently banned from operating game servers, and you have a cooldown from connecting to game servers. (Global cooldown)"
};

inline std::vector<const char*> match_rank = 
{
	"Off", "Silver 1", "Silver 2", "Silver 3", "Silver 4", "Silver elite", "Silver elite master", "Gold nova 1", "Gold nova 2", "Gold nova 3", "Gold nova master", "Master guardian 1", "Master guardian 1", "Master guardian elite", "Distinguished master guardian", "Legendary eagle", "Legendary eagle master", "Supreme master first class", "The global elite"
};

inline std::vector<const char*> dangerzone_rank = {
	"Off", "Lab rat 1", "Lab rat 2", "Sprinting hare 1", "Sprinting hare 2", "Wild scout 1", "Wild scout elite", "Hunter fox 1", "Hunter fox 2", "Hunter fox 3", "Hunter fox elite", "Timber wolf", "Ember wolf", "Wildfire wolf", "The howling alpha"
};

inline std::vector<const char*> player_model_t =
{
	"None",
	"Enforcer",
	"Soldier",
	"Ground Rebel",
	"Maximus",
	"Osiris",
	"Slingshot",
	"Dragomir",
	"Blackwolf",
	"Prof. Shahmat",
	"Rezan The Ready",
	"Doctor Romanov",
	"Mr. Muhlik",
	"Darryl Miami",
	"Darryl Silent",
	"Skullhead",
	"Royale",
	"Loudmouth",
	"Safecracker Voltzmann",
	"Little Kev",
	"Number K",
	"Getaway Sally",
	"Rezan the redshirt"
};

inline std::vector<const char*> player_model_ct =
{
	"None",
	"Seal Team 6",
	"3rd Commando",
	"Operator FBI",
	"Squadron Officer",
	"Markus Delrow",
	"Buckshot",
	"McCoy",
	"Commander Ricksaw",
	"Agent Ava",
	"Cmdr. Mae 'Dead Cold' Jamison",
	"1st lieutenant farlow",
	"John 'Van Healen' Kask",
	"Bio-Haz Specialist",
	"Sergant Bombson",
	"Chem-Haz Specialist",
	"Buckshot",
	"Mccoy"
};

inline std::vector<const char*> music_kits =
{
	"None", // 0
	"Valve", // 1
	"Valve 2", // 2
	"Crimson Assault", // 3
	"Sharpened", // 4
	"Insurgency", // 5
	"A*D*8", // 6
	"High Noon", // 7
	"Death's Head Demolition", // 8
	"Desert Fire", // 9
	"LNOE", // 10
	"Metal", // 11
	"All I Want for Christmas", // 12
	"IsoRhythm", // 13
	"For No Mankind", // 14
	"Hotline Miami", // 15
	"Total Domination", // 16
	"The Talos Principle", // 17
	"Battlepack", // 18
	"MOLOTOV", // 19
	"Uber Blasto Phone", // 20
	"Hazardous Environments", // 21
	"II-Headshot", // 22
	"The 8-Bit Kit'", // 23
	"I Am", // 24
	"Diamonds", // 25
	"Invasion!", // 26
	"Lion's Mouth", //27
	"Sponge Fingerz", // 28
	"Disgusting", // 29
	"Java Havana Funkaloo", // 30
	"Moments CSGO", // 31
	"Aggressive", // 32
	"The Good Youth", // 33
	"FREE", // 34
	"Life's Not Out To Get You", // 35
	"Backbone", // 36
	"GLA", // 37
	"III-Arena", // 38
	"EZ4ENCE", // 39
	"The Master Chief Collection", // 40
	"Scar", // 41
	"Anti Citizen", // 42
	"Bachram", // 43
	"Gunman Taco Truck", // 44
	"Eye of the Dragon", // 45
	"M.U.D.D. FORCE", // 46
	"Neo Noir", // 47
	"Bodacious", // 48
	"Drifter", // 49
	"All for Dust", // 50
	"Hades Music Kit", // 51
	"The Lowlife Pack", // 52
	"CHAIN$AW.LXADXUT.", // 53
	"Mocha Petal", // 54
	"Yellow Magic", // 55
	"Vici", // 56
	"Astro Bellum", // 57
	"Work hard，Play hard", // 58
	"KOLIBRI", // 59
	"u mad!", // 60
	"Flashbang Dance", // 61
	"Heading for the Source", // 62
	"Void", // 63
	"Shooters", // 64
	"dashstar*", // 65
	"Gothic Luxury", // 66
	"Lock Me Up", // 67
	"Hua Lian", // 68
};
