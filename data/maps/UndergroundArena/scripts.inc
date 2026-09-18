@ Underground arena. See claude-guide/features/FEATURE_ARENA_AND_HUB.md
@ Reached only through the fence in Littleroot Town (data/scripts/underground.inc).
@ Five tables fought as wager battles paid in UCOIN; each win opens the next.

UndergroundArena_MapScripts::
	.byte 0

@ ---------------------------------------------------------------- access (called from the fence)

@ Offers the buy-in once the player is trusted and has not paid yet.
Arena_EventScript_FenceOfferBuyIn::
	goto_if_set FLAG_ARENA_ACCESS, Arena_EventScript_Return
	msgbox Arena_Text_FenceMentionsArena, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_BuyInDeclined
	setvar VAR_0x8004, ARENA_BUY_IN
	special Special_Underground_TrySpendUcoins
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_CantAffordBuyIn
	setflag FLAG_ARENA_ACCESS
	msgbox Arena_Text_BuyInPaid, MSGBOX_DEFAULT
	return

Arena_EventScript_BuyInDeclined:
	msgbox Arena_Text_BuyInDeclined, MSGBOX_DEFAULT
	return

Arena_EventScript_CantAffordBuyIn:
	msgbox Arena_Text_CantAffordBuyIn, MSGBOX_DEFAULT
	return

Arena_EventScript_GoDownstairs::
	msgbox Arena_Text_FenceLeadsDown, MSGBOX_DEFAULT
	closemessage
	setdynamicwarp MAP_LITTLEROOT_TOWN, 2, 15
	warp MAP_UNDERGROUND_ARENA, 0
	waitstate
	release
	end

@ ---------------------------------------------------------------- shared

@ In: VAR_0x8008 = table. Out: VAR_RESULT = TRUE if the fight may go ahead.
Arena_EventScript_CheckCanFight:
	call Underground_EventScript_CheckLockout
	goto_if_eq VAR_RESULT, TRUE, Arena_EventScript_CantFight
	special Special_Arena_IsTierOpen
	goto_if_eq VAR_RESULT, TRUE, Arena_EventScript_Return
	msgbox Arena_Text_TableLocked, MSGBOX_DEFAULT
Arena_EventScript_CantFight:
	setvar VAR_RESULT, FALSE
	return

@ Opponent stake in VAR_WAGER_OPP_*; sets the UCOIN flag and asks for the player's stake.
Arena_EventScript_TakeBet:
	setflag FLAG_WAGER_IN_UCOIN
	call Wager_EventScript_ChooseStake
	return

@ In: VAR_RESULT = wager result from Wager_EventScript_Settle, VAR_0x8008 = table.
Arena_EventScript_AfterFight:
	copyvar VAR_0x8004, VAR_RESULT
	call Ascension_EventScript_ArenaBounty
	special Special_Arena_RecordResult
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_Return
	goto_if_eq VAR_0x8008, ARENA_TIER_COUNT, Arena_EventScript_BossBeaten
	msgbox Arena_Text_NextTableOpen, MSGBOX_DEFAULT
	return

Arena_EventScript_BossBeaten:
	playfanfare MUS_OBTAIN_BADGE
	msgbox Arena_Text_YouOwnThePlace, MSGBOX_DEFAULT
	waitfanfare
	return

Arena_EventScript_Declined:
	msgbox Arena_Text_Declined, MSGBOX_DEFAULT
Arena_EventScript_End:
	release
	end

Arena_EventScript_Return:
	return

@ ---------------------------------------------------------------- host

UndergroundArena_EventScript_Host::
	lock
	faceplayer
	special Special_Arena_BufferStatus
	msgbox Arena_Text_HostStatus, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------- tables
@ Each table: gate, set the house stake, take the bet, fight, settle, record.
@ wagerbattle must stay at the top level of the script (not inside a call).

UndergroundArena_EventScript_Table1::
	lock
	faceplayer
	setvar VAR_0x8008, 1
	call Arena_EventScript_CheckCanFight
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_End
	msgbox Arena_Text_Table1Intro, MSGBOX_DEFAULT
	setvar VAR_WAGER_OPP_MONEY, 150
	setvar VAR_WAGER_OPP_SPECIES, SPECIES_NONE
	setvar VAR_WAGER_OPP_LEVEL, 0
	call Arena_EventScript_TakeBet
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_Declined
	closemessage
	wagerbattle TRAINER_ARENA_1, Arena_Text_Table1Lost, Arena_Text_Table1Won
	call Wager_EventScript_Settle
	setvar VAR_0x8008, 1
	call Arena_EventScript_AfterFight
	release
	end

UndergroundArena_EventScript_Table2::
	lock
	faceplayer
	setvar VAR_0x8008, 2
	call Arena_EventScript_CheckCanFight
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_End
	msgbox Arena_Text_Table2Intro, MSGBOX_DEFAULT
	setvar VAR_WAGER_OPP_MONEY, 400
	setvar VAR_WAGER_OPP_SPECIES, SPECIES_NONE
	setvar VAR_WAGER_OPP_LEVEL, 0
	call Arena_EventScript_TakeBet
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_Declined
	closemessage
	wagerbattle TRAINER_ARENA_2, Arena_Text_Table2Lost, Arena_Text_Table2Won
	call Wager_EventScript_Settle
	setvar VAR_0x8008, 2
	call Arena_EventScript_AfterFight
	release
	end

UndergroundArena_EventScript_Table3::
	lock
	faceplayer
	setvar VAR_0x8008, 3
	call Arena_EventScript_CheckCanFight
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_End
	msgbox Arena_Text_Table3Intro, MSGBOX_DEFAULT
	setvar VAR_WAGER_OPP_MONEY, 900
	setvar VAR_WAGER_OPP_SPECIES, SPECIES_BAGON
	setvar VAR_WAGER_OPP_LEVEL, 30
	call Arena_EventScript_TakeBet
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_Declined
	closemessage
	wagerbattle TRAINER_ARENA_3, Arena_Text_Table3Lost, Arena_Text_Table3Won
	call Wager_EventScript_Settle
	setvar VAR_0x8008, 3
	call Arena_EventScript_AfterFight
	release
	end

UndergroundArena_EventScript_Table4::
	lock
	faceplayer
	setvar VAR_0x8008, 4
	call Arena_EventScript_CheckCanFight
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_End
	msgbox Arena_Text_Table4Intro, MSGBOX_DEFAULT
	setvar VAR_WAGER_OPP_MONEY, 2000
	setvar VAR_WAGER_OPP_SPECIES, SPECIES_NONE
	setvar VAR_WAGER_OPP_LEVEL, 0
	call Arena_EventScript_TakeBet
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_Declined
	closemessage
	wagerbattle TRAINER_ARENA_4, Arena_Text_Table4Lost, Arena_Text_Table4Won
	call Wager_EventScript_Settle
	setvar VAR_0x8008, 4
	call Arena_EventScript_AfterFight
	release
	end

UndergroundArena_EventScript_Table5::
	lock
	faceplayer
	setvar VAR_0x8008, 5
	call Arena_EventScript_CheckCanFight
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_End
	msgbox Arena_Text_Table5Intro, MSGBOX_DEFAULT
	setvar VAR_WAGER_OPP_MONEY, 5000
	setvar VAR_WAGER_OPP_SPECIES, SPECIES_GARCHOMP
	setvar VAR_WAGER_OPP_LEVEL, 50
	call Arena_EventScript_TakeBet
	goto_if_eq VAR_RESULT, FALSE, Arena_EventScript_Declined
	closemessage
	wagerbattle TRAINER_ARENA_5, Arena_Text_Table5Lost, Arena_Text_Table5Won
	call Wager_EventScript_Settle
	setvar VAR_0x8008, 5
	call Arena_EventScript_AfterFight
	release
	end

@ ---------------------------------------------------------------- text

Arena_Text_FenceMentionsArena:
	.string "You've been straight with me.\p"
	.string "There's a room downstairs where\n"
	.string "people fight for real stakes.\p"
	.string "Entry's 100 UCOIN, once.\n"
	.string "Interested?$"
Arena_Text_BuyInPaid:
	.string "Good. Ask me any time and I'll\n"
	.string "take you down.$"
Arena_Text_BuyInDeclined:
	.string "Offer stands. You know where I am.$"
Arena_Text_CantAffordBuyIn:
	.string "Come back with 100 UCOIN.$"
Arena_Text_FenceLeadsDown:
	.string "Follow me. Keep your head down.$"

Arena_Text_TableLocked:
	.string "Beat the lower tables first.\n"
	.string "Nobody skips the line down here.$"
Arena_Text_NextTableOpen:
	.string "Word travels. The next table\n"
	.string "will take your bet now.$"
Arena_Text_YouOwnThePlace:
	.string "You beat the House.\p"
	.string "Nobody down here will forget\n"
	.string "your name.$"
Arena_Text_Declined:
	.string "No stake, no fight.$"

Arena_Text_HostStatus:
	.string "Welcome to the pit.\p"
	.string "Five tables. Beat one, the next\n"
	.string "opens. Stakes are paid in UCOIN.\p"
	.string "Bet coin, a Pokémon, or both.\n"
	.string "Lose, and it's gone.\p"
	.string "Tables beaten: {STR_VAR_1} of {STR_VAR_3}.\n"
	.string "You hold {STR_VAR_2} UCOIN.$"

Arena_Text_Table1Intro:
	.string "Fresh meat. My table pays\n"
	.string "150 UCOIN. What's your stake?$"
Arena_Text_Table1Lost:
	.string "Beginner's luck, that's all.$"
Arena_Text_Table1Won:
	.string "Told you. Fresh meat.$"

Arena_Text_Table2Intro:
	.string "I break things for a living.\n"
	.string "400 UCOIN says I break you.$"
Arena_Text_Table2Lost:
	.string "Hrmph. Solid hit.$"
Arena_Text_Table2Won:
	.string "Broken. As promised.$"

Arena_Text_Table3Intro:
	.string "Fufu… 900 UCOIN and a Bagon\n"
	.string "rest on this table. Dare you?$"
Arena_Text_Table3Lost:
	.string "The spirits turned on me…$"
Arena_Text_Table3Won:
	.string "The spirits keep what they take.$"

Arena_Text_Table4Intro:
	.string "I don't lose. 2000 UCOIN on the\n"
	.string "table. Match it or walk.$"
Arena_Text_Table4Lost:
	.string "…Impossible.$"
Arena_Text_Table4Won:
	.string "Like I said.$"

Arena_Text_Table5Intro:
	.string "So you're the one climbing my\n"
	.string "tables.\p"
	.string "5000 UCOIN and my Garchomp.\n"
	.string "The House always wins.$"
Arena_Text_Table5Lost:
	.string "…The House lost.$"
Arena_Text_Table5Won:
	.string "The House always wins.$"
