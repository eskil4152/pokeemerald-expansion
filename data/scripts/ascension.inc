@ Ascension, phases 2-5. See claude-guide/features/FEATURE_ASCENSION.md
@
@ The mystic (League lobby and Sandbox Hub) takes the Champion to the Shrine.
@ The Oracle at the Shrine performs ascensions and judges trials.
@ Visiting champions wait in the League lobby. All logic is in src/ascension.c.
@
@ Battle scripts here use wagerbattle (no whiteout, VAR_RESULT = TRUE if lost),
@ which must run at top level: everything below is reached by goto, not call.

@ ---------------------------------------------------------------- Mystic

Ascension_EventScript_Mystic::
	lock
	faceplayer
	goto_if_unset FLAG_IS_CHAMPION, Ascension_EventScript_MysticNotChampion
	checkquest QUEST_ASCENSION_I
	call_if_eq VAR_RESULT, QUEST_STAGE_NONE, Ascension_EventScript_MysticIntro
	msgbox Ascension_Text_MysticOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Ascension_EventScript_MysticDeclined
	msgbox Ascension_Text_MysticClose, MSGBOX_DEFAULT
	closemessage
	warp MAP_ASCENSION_SHRINE, 12, 13
	waitstate
	release
	end

Ascension_EventScript_MysticIntro:
	msgbox Ascension_Text_MysticIntro, MSGBOX_DEFAULT
	startquest QUEST_ASCENSION_I
	return

Ascension_EventScript_MysticNotChampion:
	msgbox Ascension_Text_MysticNotChampion, MSGBOX_DEFAULT
	release
	end

Ascension_EventScript_MysticDeclined:
	msgbox Ascension_Text_MysticDeclined, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------- Oracle

Ascension_EventScript_Oracle::
	lock
	faceplayer
	goto_if_set FLAG_ASCENSION_MET_ORACLE, Ascension_EventScript_OracleMenu
	setflag FLAG_ASCENSION_MET_ORACLE
	msgbox Ascension_Text_OracleIntro, MSGBOX_DEFAULT
Ascension_EventScript_OracleMenu:
	message Ascension_Text_OracleWhatDoYouSeek
	waitmessage
	dynmultichoice 0, 0, FALSE, 4, 0, DYN_MULTICHOICE_CB_NONE, Ascension_Text_OptionAscend, Ascension_Text_OptionTrial, Ascension_Text_OptionStanding, Ascension_Text_OptionLeave
	switch VAR_RESULT
	case 0, Ascension_EventScript_Ascend
	case 1, Ascension_EventScript_Trial
	case 2, Ascension_EventScript_Standing
	msgbox Ascension_Text_OracleBye, MSGBOX_DEFAULT
	release
	end

Ascension_EventScript_Standing:
	special Special_Ascension_BufferStatus
	msgbox Ascension_Text_Standing, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

@ ---------------------------------------------------------------- Ascending a Pokémon

Ascension_EventScript_Ascend:
	msgbox Ascension_Text_ChooseChallenger, MSGBOX_DEFAULT
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Ascension_EventScript_OracleMenu
	special Special_Ascension_CheckRequirements
	switch VAR_RESULT
	case ASC_REQ_EGG, Ascension_EventScript_ReqEgg
	case ASC_REQ_MAXED, Ascension_EventScript_ReqMaxed
	case ASC_REQ_RANK, Ascension_EventScript_ReqRank
	case ASC_REQ_LEVEL, Ascension_EventScript_ReqLevel
	case ASC_REQ_FRIENDSHIP, Ascension_EventScript_ReqFriendship
	case ASC_REQ_WINS, Ascension_EventScript_ReqWins
	case ASC_REQ_SHARDS, Ascension_EventScript_ReqShards
	msgbox Ascension_Text_ConfirmRitual, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Ascension_EventScript_OracleMenu
	msgbox Ascension_Text_GuardianAppears, MSGBOX_DEFAULT
	closemessage
	setwildbattle SPECIES_SHAYMIN, 1    @ placeholder, replaced by the guardian below
	special Special_Ascension_StartGuardianBattle
	dowildbattle
	special Special_Ascension_FinishGuardianBattle
	copyvar VAR_0x8006, VAR_RESULT
	special HealPlayerParty
	goto_if_eq VAR_0x8006, TRUE, Ascension_EventScript_Ascended
	msgbox Ascension_Text_GuardianWon, MSGBOX_DEFAULT
	release
	end

Ascension_EventScript_Ascended:
	playfanfare MUS_OBTAIN_BADGE
	msgbox Ascension_Text_Ascended, MSGBOX_DEFAULT
	waitfanfare
	release
	end

Ascension_EventScript_ReqEgg:
	msgbox Ascension_Text_ReqEgg, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_ReqMaxed:
	msgbox Ascension_Text_ReqMaxed, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_ReqRank:
	msgbox Ascension_Text_ReqRank, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_ReqLevel:
	msgbox Ascension_Text_ReqLevel, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_ReqFriendship:
	msgbox Ascension_Text_ReqFriendship, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_ReqWins:
	msgbox Ascension_Text_ReqWins, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_ReqShards:
	msgbox Ascension_Text_ReqShards, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

@ ---------------------------------------------------------------- Trials

Ascension_EventScript_Trial:
	special Special_Ascension_GetTrialState
	switch VAR_RESULT
	case ASC_TRIAL_ALL_DONE, Ascension_EventScript_TrialAllDone
	case ASC_TRIAL_DETHRONED, Ascension_EventScript_TrialDethroned
	case ASC_TRIAL_READY, Ascension_EventScript_TrialPassed
	case ASC_TRIAL_BATTLE, Ascension_EventScript_TrialBattle
	switch VAR_0x8005
	case 0, Ascension_EventScript_TrialIPending
	case 1, Ascension_EventScript_TrialIIPending
	goto Ascension_EventScript_TrialIIIPending

Ascension_EventScript_TrialIPending:
	msgbox Ascension_Text_TrialI, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_TrialIIPending:
	msgbox Ascension_Text_TrialII, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_TrialIIIPending:
	msgbox Ascension_Text_TrialIII, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_TrialAllDone:
	msgbox Ascension_Text_TrialAllDone, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_TrialDethroned:
	msgbox Ascension_Text_TrialDethroned, MSGBOX_DEFAULT
	goto Ascension_EventScript_OracleMenu

Ascension_EventScript_TrialPassed:
	special Special_Ascension_AdvanceRank
	playfanfare MUS_OBTAIN_BADGE
	msgbox Ascension_Text_RankUp, MSGBOX_DEFAULT
	waitfanfare
	giveitem ITEM_ASCENSION_SHARD, ASCENSION_SHARDS_PER_RANK
	goto_if_eq VAR_ASCENSION_RANK, ASCENSION_MAX_RANK, Ascension_EventScript_Apotheosis
	msgbox Ascension_Text_RankUpAfter, MSGBOX_DEFAULT
	release
	end

Ascension_EventScript_Apotheosis:
	msgbox Ascension_Text_Apotheosis, MSGBOX_DEFAULT
	release
	end

Ascension_EventScript_TrialBattle:
	goto_if_eq VAR_0x8005, 4, Ascension_EventScript_AscendedOne
	msgbox Ascension_Text_GauntletExplain, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Ascension_EventScript_OracleMenu
	setvar VAR_ASCENSION_NO_BAG, NO_BAG_AGAINST_TRAINER
	msgbox Ascension_Text_GauntletSidney, MSGBOX_DEFAULT
	closemessage
	wagerbattle TRAINER_SIDNEY, Ascension_Text_GauntletDefeat
	goto_if_eq VAR_RESULT, TRUE, Ascension_EventScript_GauntletLost
	msgbox Ascension_Text_GauntletPhoebe, MSGBOX_DEFAULT
	closemessage
	wagerbattle TRAINER_PHOEBE, Ascension_Text_GauntletDefeat
	goto_if_eq VAR_RESULT, TRUE, Ascension_EventScript_GauntletLost
	msgbox Ascension_Text_GauntletGlacia, MSGBOX_DEFAULT
	closemessage
	wagerbattle TRAINER_GLACIA, Ascension_Text_GauntletDefeat
	goto_if_eq VAR_RESULT, TRUE, Ascension_EventScript_GauntletLost
	msgbox Ascension_Text_GauntletDrake, MSGBOX_DEFAULT
	closemessage
	wagerbattle TRAINER_DRAKE, Ascension_Text_GauntletDefeat
	goto_if_eq VAR_RESULT, TRUE, Ascension_EventScript_GauntletLost
	msgbox Ascension_Text_GauntletWallace, MSGBOX_DEFAULT
	closemessage
	wagerbattle TRAINER_WALLACE, Ascension_Text_GauntletDefeat
	goto_if_eq VAR_RESULT, TRUE, Ascension_EventScript_GauntletLost
	setvar VAR_ASCENSION_NO_BAG, NO_BAG_RESTRICTION
	special HealPlayerParty
	msgbox Ascension_Text_GauntletCleared, MSGBOX_DEFAULT
	goto Ascension_EventScript_TrialPassed

Ascension_EventScript_GauntletLost:
	setvar VAR_ASCENSION_NO_BAG, NO_BAG_RESTRICTION
	msgbox Ascension_Text_GauntletLost, MSGBOX_DEFAULT
	release
	end

Ascension_EventScript_AscendedOne:
	msgbox Ascension_Text_AscendedExplain, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Ascension_EventScript_OracleMenu
	msgbox Ascension_Text_AscendedArrives, MSGBOX_DEFAULT
	closemessage
	wagerbattle TRAINER_ASCENDED_ONE, Ascension_Text_AscendedDefeat
	goto_if_eq VAR_RESULT, TRUE, Ascension_EventScript_AscendedLost
	special HealPlayerParty
	msgbox Ascension_Text_AscendedBeaten, MSGBOX_DEFAULT
	goto Ascension_EventScript_TrialPassed

Ascension_EventScript_AscendedLost:
	msgbox Ascension_Text_AscendedLost, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------- Visiting champions

@ Object in the League lobby, graphics from VAR_OBJ_GFX_ID_0 (Special_Ascension_SetupLeagueVisitor).
Ascension_EventScript_Visitor::
	lock
	faceplayer
	special Special_Ascension_BufferVisitor
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_set FLAG_ASCENSION_DETHRONED, Ascension_EventScript_VisitorHolder
	msgbox Ascension_Text_VisitorChallenge, MSGBOX_YESNO
	goto Ascension_EventScript_VisitorAnswer

Ascension_EventScript_VisitorHolder:
	msgbox Ascension_Text_VisitorHolder, MSGBOX_YESNO
Ascension_EventScript_VisitorAnswer:
	goto_if_eq VAR_RESULT, NO, Ascension_EventScript_VisitorLater
	msgbox Ascension_Text_VisitorBegin, MSGBOX_DEFAULT
	closemessage
	switch VAR_0x8008
	case ASCENSION_VISITOR_RED, Ascension_EventScript_BattleRed
	case ASCENSION_VISITOR_LEAF, Ascension_EventScript_BattleLeaf
	case ASCENSION_VISITOR_BLUE, Ascension_EventScript_BattleBlue
	case ASCENSION_VISITOR_LANCE, Ascension_EventScript_BattleLance
	case ASCENSION_VISITOR_STEVEN, Ascension_EventScript_BattleSteven
	case ASCENSION_VISITOR_WALLACE, Ascension_EventScript_BattleWallace
	case ASCENSION_VISITOR_CYNTHIA, Ascension_EventScript_BattleCynthia
	release
	end

Ascension_EventScript_VisitorLater:
	msgbox Ascension_Text_VisitorLater, MSGBOX_DEFAULT
	release
	end

Ascension_EventScript_BattleRed:
	wagerbattle TRAINER_RED, Ascension_Text_VisitorDefeat
	goto Ascension_EventScript_AfterVisitorBattle

Ascension_EventScript_BattleLeaf:
	wagerbattle TRAINER_LEAF, Ascension_Text_VisitorDefeat
	goto Ascension_EventScript_AfterVisitorBattle

Ascension_EventScript_BattleBlue:
	wagerbattle TRAINER_VISITOR_BLUE, Ascension_Text_VisitorDefeat
	goto Ascension_EventScript_AfterVisitorBattle

Ascension_EventScript_BattleLance:
	wagerbattle TRAINER_VISITOR_LANCE, Ascension_Text_VisitorDefeat
	goto Ascension_EventScript_AfterVisitorBattle

Ascension_EventScript_BattleSteven:
	wagerbattle TRAINER_VISITOR_STEVEN, Ascension_Text_VisitorDefeat
	goto Ascension_EventScript_AfterVisitorBattle

Ascension_EventScript_BattleWallace:
	wagerbattle TRAINER_VISITOR_WALLACE, Ascension_Text_VisitorDefeat
	goto Ascension_EventScript_AfterVisitorBattle

Ascension_EventScript_BattleCynthia:
	wagerbattle TRAINER_VISITOR_CYNTHIA, Ascension_Text_VisitorDefeat
	goto Ascension_EventScript_AfterVisitorBattle

Ascension_EventScript_AfterVisitorBattle:
	copyvar VAR_0x8004, VAR_RESULT
	special Special_Ascension_BufferVisitor
	special Special_Ascension_OnVisitorBattle
	goto_if_eq VAR_0x8004, TRUE, Ascension_EventScript_VisitorTookTitle
	msgbox Ascension_Text_VisitorBeaten, MSGBOX_DEFAULT
	giveitem ITEM_ASCENSION_SHARD, ASCENSION_VISITOR_BOUNTY
	msgbox Ascension_Text_VisitorLeaves, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_LEAGUE_VISITOR
	fadescreen FADE_FROM_BLACK
	release
	end

Ascension_EventScript_VisitorTookTitle:
	msgbox Ascension_Text_VisitorTookTitle, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------- World

@ Called from Arena_EventScript_AfterFight. In: VAR_0x8004 = wager result, VAR_0x8008 = table.
Ascension_EventScript_ArenaBounty::
	goto_if_ne VAR_0x8004, WAGER_RESULT_WON, Ascension_EventScript_Return
	goto_if_ne VAR_0x8008, ARENA_TIER_COUNT, Ascension_EventScript_Return
	special Special_Ascension_TryWeeklyBounty
	goto_if_eq VAR_RESULT, FALSE, Ascension_EventScript_Return
	msgbox Ascension_Text_ArenaBounty, MSGBOX_DEFAULT
	giveitem ITEM_ASCENSION_SHARD, ASCENSION_WEEKLY_BOUNTY
	return

@ Called from Common_EventScript_PkmnCenterNurse. Sometimes shares a rumour about the player.
Ascension_EventScript_NurseRumour::
	special Special_Ascension_BufferRumour
	goto_if_eq VAR_RESULT, FALSE, Ascension_EventScript_Return
	msgbox gStringVar4, MSGBOX_DEFAULT
	return

Ascension_EventScript_Return:
	return

@ ---------------------------------------------------------------- Text

Ascension_Text_MysticNotChampion:
	.string "Not yet. Come back when the League\n"
	.string "calls you Champion.$"

Ascension_Text_MysticIntro:
	.string "Champion. The title is only a door.\p"
	.string "Beyond it lies a Shrine where Pokémon\n"
	.string "shed their limits, and trainers\l"
	.string "become something more.$"

Ascension_Text_MysticOffer:
	.string "Shall I take you to the Shrine?$"

Ascension_Text_MysticClose:
	.string "Close your eyes.$"

Ascension_Text_MysticDeclined:
	.string "The Shrine will wait.\n"
	.string "It has always waited.$"

Ascension_Text_OracleIntro:
	.string "I am the Oracle of this Shrine.\p"
	.string "A Pokémon may ascend here. It must\n"
	.string "love you completely, and it must have\l"
	.string "won enough battles at your side.\p"
	.string "Then it faces a guardian alone. If\n"
	.string "it wins, it rises, and the shards you\l"
	.string "offer are spent.\p"
	.string "You earn shards by passing my trials.\n"
	.string "Each trial raises your rank.$"

Ascension_Text_OracleWhatDoYouSeek:
	.string "What do you seek?$"

Ascension_Text_OptionAscend:
	.string "Ascend a Pokémon$"

Ascension_Text_OptionTrial:
	.string "Present my trial$"

Ascension_Text_OptionStanding:
	.string "My standing$"

Ascension_Text_OptionLeave:
	.string "Leave$"

Ascension_Text_OracleBye:
	.string "The stairs will take you back.$"

Ascension_Text_Standing:
	.string "You stand at {STR_VAR_1}.\p"
	.string "You have held the title for\n"
	.string "{STR_VAR_2} days, and carry {STR_VAR_3} shards.$"

Ascension_Text_ChooseChallenger:
	.string "Which Pokémon will ascend?$"

Ascension_Text_ReqEgg:
	.string "An Egg cannot fight a guardian.$"

Ascension_Text_ReqMaxed:
	.string "{STR_VAR_1} has already reached\n"
	.string "the summit.$"

Ascension_Text_ReqRank:
	.string "Tier {STR_VAR_3} needs a trainer of\n"
	.string "{STR_VAR_2}. Pass more trials.$"

Ascension_Text_ReqLevel:
	.string "{STR_VAR_1} must reach level\n"
	.string "{STR_VAR_2} first.$"

Ascension_Text_ReqFriendship:
	.string "{STR_VAR_1} does not trust you\n"
	.string "completely yet.$"

Ascension_Text_ReqWins:
	.string "{STR_VAR_1} needs {STR_VAR_2} more\n"
	.string "wins against trainers.$"

Ascension_Text_ReqShards:
	.string "Tier {STR_VAR_3} asks an offering of\n"
	.string "{STR_VAR_2} shards.$"

Ascension_Text_ConfirmRitual:
	.string "{STR_VAR_1} will face {STR_VAR_2}\n"
	.string "alone. No items. No retreat.\p"
	.string "If it wins, it rises to tier\n"
	.string "{STR_VAR_3}. Begin?$"

Ascension_Text_GuardianAppears:
	.string "The guardian answers.$"

Ascension_Text_GuardianWon:
	.string "The guardian stands. Your shards are\n"
	.string "untouched. Return when ready.$"

Ascension_Text_Ascended:
	.string "{STR_VAR_1} ascended to\n"
	.string "tier {STR_VAR_2}!$"

Ascension_Text_TrialI:
	.string "Your first trial: hold the Champion\n"
	.string "title for seven days.$"

Ascension_Text_TrialII:
	.string "Your second trial: own the\n"
	.string "Underground Arena. Clear every table.$"

Ascension_Text_TrialIII:
	.string "Your third trial: defeat a Champion\n"
	.string "who comes to take your title.\p"
	.string "They come to the League lobby\n"
	.string "every seven days.$"

Ascension_Text_TrialAllDone:
	.string "There are no more trials.\n"
	.string "There is no one above you.$"

Ascension_Text_TrialDethroned:
	.string "You have lost the title. Take it back\n"
	.string "at the League before you ask me\l"
	.string "for anything.$"

Ascension_Text_RankUp:
	.string "You are now {STR_VAR_1}.$"

Ascension_Text_RankUpAfter:
	.string "Stronger Pokémon may now ascend.\n"
	.string "The world will notice.$"

Ascension_Text_Apotheosis:
	.string "...\p"
	.string "I have served this Shrine for a\n"
	.string "thousand years, waiting for this.\p"
	.string "Go. The world is yours.$"

Ascension_Text_GauntletExplain:
	.string "Your fourth trial: five battles in a\n"
	.string "row. No healing between them, and\l"
	.string "no items. Lose once and it ends.\p"
	.string "Are you ready?$"

Ascension_Text_GauntletSidney:
	.string "The first shadow takes shape.$"

Ascension_Text_GauntletPhoebe:
	.string "The second shadow.$"

Ascension_Text_GauntletGlacia:
	.string "The third shadow.$"

Ascension_Text_GauntletDrake:
	.string "The fourth shadow.$"

Ascension_Text_GauntletWallace:
	.string "The last shadow.$"

Ascension_Text_GauntletDefeat:
	.string "The shadow fades.$"

Ascension_Text_GauntletCleared:
	.string "The shadows are gone.$"

Ascension_Text_GauntletLost:
	.string "The gauntlet ends. Rest, and\n"
	.string "come back.$"

Ascension_Text_AscendedExplain:
	.string "Your last trial.\p"
	.string "Another trainer climbed these stairs\n"
	.string "long ago, and never came down.\p"
	.string "Every one of their Pokémon has\n"
	.string "ascended fully. Will you face them?$"

Ascension_Text_AscendedArrives:
	.string "The air goes still.$"

Ascension_Text_AscendedDefeat:
	.string "...So it is your turn.$"

Ascension_Text_AscendedBeaten:
	.string "The one who came before you bows,\n"
	.string "and is gone.$"

Ascension_Text_AscendedLost:
	.string "Not yet. The summit waits.$"

Ascension_Text_VisitorChallenge:
	.string "{STR_VAR_1}: So you're the Champion\n"
	.string "everyone keeps talking about.\p"
	.string "Your dominance must be known.\n"
	.string "Battle me, here, now.$"

Ascension_Text_VisitorHolder:
	.string "{STR_VAR_1}: The title is mine now.\n"
	.string "Want it back?$"

Ascension_Text_VisitorBegin:
	.string "{STR_VAR_1}: Don't hold back.$"

Ascension_Text_VisitorLater:
	.string "{STR_VAR_1}: Don't keep me waiting\n"
	.string "too long.$"

Ascension_Text_VisitorDefeat:
	.string "The rumours were true.$"

Ascension_Text_VisitorBeaten:
	.string "{STR_VAR_1}: You earned your title.\n"
	.string "Take this.$"

Ascension_Text_VisitorLeaves:
	.string "{STR_VAR_1}: I'll tell them what\n"
	.string "I saw here.$"

Ascension_Text_VisitorTookTitle:
	.string "{STR_VAR_1}: The title is mine.\n"
	.string "Come take it back when you can.$"

Ascension_Text_ArenaBounty:
	.string "The house pays tribute to the\n"
	.string "Champion. Once a week, as agreed.$"
