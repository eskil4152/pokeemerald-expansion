@ Companions. See claude-guide/features/FEATURE_LOUNGE.md
@
@ The attendant in the League lobby leads to the Champions' Lounge (candidates
@ in six slots) and, once married, the Champions' Residence (the spouse).
@ Outings wait at Lilycove, Mossdeep and the Ascension Shrine. All logic is in
@ src/companion.c; VAR_0x8008 holds the selected candidate.
@
@ Lounge battles use wagerbattle, so every menu path below is reached by goto.
@ VAR_0x800A: which menu to return to (0 = lounge, 1 = residence).

@ ---------------------------------------------------------------- Attendant

Companion_EventScript_Attendant::
	lock
	faceplayer
	goto_if_unset FLAG_IS_CHAMPION, Companion_EventScript_AttendantNotChampion
	message Companion_Text_AttendantWelcome
	waitmessage
	dynmultichoice 0, 0, FALSE, 4, 0, DYN_MULTICHOICE_CB_NONE, Companion_Text_OptionLounge, Companion_Text_OptionResidence, Companion_Text_OptionRing, Companion_Text_OptionNothing
	switch VAR_RESULT
	case 0, Companion_EventScript_GoLounge
	case 1, Companion_EventScript_GoResidence
	case 2, Companion_EventScript_BuyRing
	msgbox Companion_Text_AttendantBye, MSGBOX_DEFAULT
	release
	end

Companion_EventScript_AttendantNotChampion:
	msgbox Companion_Text_AttendantNotChampion, MSGBOX_DEFAULT
	release
	end

Companion_EventScript_GoLounge:
	msgbox Companion_Text_AttendantLounge, MSGBOX_DEFAULT
	closemessage
	warp MAP_CHAMPIONS_LOUNGE, 0
	waitstate
	release
	end

Companion_EventScript_GoResidence:
	special Special_Companion_SelectSpouse
	goto_if_eq VAR_RESULT, FALSE, Companion_EventScript_NoResidence
	msgbox Companion_Text_AttendantResidence, MSGBOX_DEFAULT
	closemessage
	warp MAP_CHAMPIONS_RESIDENCE, 0
	waitstate
	release
	end

Companion_EventScript_NoResidence:
	msgbox Companion_Text_AttendantNoResidence, MSGBOX_DEFAULT
	release
	end

Companion_EventScript_BuyRing:
	message Companion_Text_AttendantRing
	waitmessage
	pokemart Companion_Mart_Ring
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Companion_Mart_Ring:
	.2byte ITEM_PROMISE_RING
	pokemartlistend

@ ---------------------------------------------------------------- Lounge

Companion_EventScript_Slot1::
	setvar VAR_0x8004, 0
	goto Companion_EventScript_LoungeCandidate
Companion_EventScript_Slot2::
	setvar VAR_0x8004, 1
	goto Companion_EventScript_LoungeCandidate
Companion_EventScript_Slot3::
	setvar VAR_0x8004, 2
	goto Companion_EventScript_LoungeCandidate
Companion_EventScript_Slot4::
	setvar VAR_0x8004, 3
	goto Companion_EventScript_LoungeCandidate
Companion_EventScript_Slot5::
	setvar VAR_0x8004, 4
	goto Companion_EventScript_LoungeCandidate
Companion_EventScript_Slot6::
	setvar VAR_0x8004, 5
	goto Companion_EventScript_LoungeCandidate

Companion_EventScript_LoungeCandidate:
	lock
	faceplayer
	special Special_Companion_SelectSlot
	setvar VAR_0x800A, 0
Companion_EventScript_CandidateMenu:
	message Companion_Text_WhatToDo
	waitmessage
	dynmultichoice 0, 0, FALSE, 6, 0, DYN_MULTICHOICE_CB_NONE, Companion_Text_OptionTalk, Companion_Text_OptionGift, Companion_Text_OptionBattle, Companion_Text_OptionGoOut, Companion_Text_OptionPropose, Companion_Text_OptionLeave
	switch VAR_RESULT
	case 0, Companion_EventScript_Talk
	case 1, Companion_EventScript_Gift
	case 2, Companion_EventScript_Battle
	case 3, Companion_EventScript_GoOut
	case 4, Companion_EventScript_Propose
	release
	end

Companion_EventScript_ReturnToMenu:
	goto_if_eq VAR_0x800A, 1, Companion_EventScript_SpouseMenu
	goto Companion_EventScript_CandidateMenu

Companion_EventScript_Talk:
	special Special_Companion_Talk
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_Gift:
	special Special_Companion_PrepareGift
	switch VAR_RESULT
	case COMPANION_GIFT_ALREADY, Companion_EventScript_GiftAlready
	case COMPANION_GIFT_NOTHING, Companion_EventScript_GiftNothing
	msgbox Companion_Text_GiftConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Companion_EventScript_ReturnToMenu
	special Special_Companion_GiveGift
	switch VAR_RESULT
	case 2, Companion_EventScript_GiftDelighted
	case 1, Companion_EventScript_GiftLiked
	msgbox Companion_Text_GiftPolite, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_GiftDelighted:
	playfanfare MUS_LEVEL_UP
	msgbox Companion_Text_GiftDelighted, MSGBOX_DEFAULT
	waitfanfare
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_GiftLiked:
	msgbox Companion_Text_GiftLiked, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_GiftAlready:
	msgbox Companion_Text_GiftAlready, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_GiftNothing:
	msgbox Companion_Text_GiftNothing, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_Battle:
	special Special_Companion_CanBattle
	goto_if_eq VAR_RESULT, FALSE, Companion_EventScript_BattleTomorrow
	msgbox Companion_Text_BattleAccept, MSGBOX_DEFAULT
	closemessage
	switch VAR_0x8008
	case COMPANION_CYNTHIA, Companion_EventScript_BattleCynthia
	case COMPANION_ROXANNE, Companion_EventScript_BattleRoxanne
	case COMPANION_FLANNERY, Companion_EventScript_BattleFlannery
	case COMPANION_WINONA, Companion_EventScript_BattleWinona
	case COMPANION_PHOEBE, Companion_EventScript_BattlePhoebe
	case COMPANION_GLACIA, Companion_EventScript_BattleGlacia
	case COMPANION_STEVEN, Companion_EventScript_BattleSteven
	case COMPANION_WALLACE, Companion_EventScript_BattleWallace
	case COMPANION_BRAWLY, Companion_EventScript_BattleBrawly
	case COMPANION_SIDNEY, Companion_EventScript_BattleSidney
	case COMPANION_LANCE, Companion_EventScript_BattleLance
	release
	end

Companion_EventScript_BattleTomorrow:
	msgbox Companion_Text_BattleTomorrow, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_BattleCynthia:
	wagerbattle TRAINER_VISITOR_CYNTHIA, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleRoxanne:
	wagerbattle TRAINER_ROXANNE_5, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleFlannery:
	wagerbattle TRAINER_FLANNERY_5, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleWinona:
	wagerbattle TRAINER_WINONA_5, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattlePhoebe:
	wagerbattle TRAINER_PHOEBE, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleGlacia:
	wagerbattle TRAINER_GLACIA, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleSteven:
	wagerbattle TRAINER_STEVEN, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleWallace:
	wagerbattle TRAINER_WALLACE, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleBrawly:
	wagerbattle TRAINER_BRAWLY_5, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleSidney:
	wagerbattle TRAINER_SIDNEY, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle
Companion_EventScript_BattleLance:
	wagerbattle TRAINER_VISITOR_LANCE, Companion_Text_BattleDefeat
	goto Companion_EventScript_AfterBattle

Companion_EventScript_AfterBattle:
	special Special_Companion_AfterBattle
	special HealPlayerParty
	msgbox Companion_Text_BattleAfter, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------- Outings

Companion_EventScript_GoOut:
	special Special_Companion_GetOutingState
	switch VAR_RESULT
	case COMPANION_OUTING_NOT_READY, Companion_EventScript_OutingNotReady
	case COMPANION_OUTING_NEEDS_RING, Companion_EventScript_OutingNeedsRing
	case COMPANION_OUTING_NEEDS_RANK, Companion_EventScript_OutingNeedsRank
	case COMPANION_OUTING_DONE, Companion_EventScript_OutingDone
	switch VAR_0x8005
	case COMPANION_OUTING_LILYCOVE, Companion_EventScript_AskLilycove
	case COMPANION_OUTING_MOSSDEEP, Companion_EventScript_AskMossdeep
	case COMPANION_OUTING_CONFESSION, Companion_EventScript_AskConfession
	goto Companion_EventScript_AskCeremony

Companion_EventScript_OutingNotReady:
	msgbox Companion_Text_OutingNotReady, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_OutingNeedsRing:
	msgbox Companion_Text_OutingNeedsRing, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_OutingNeedsRank:
	msgbox Companion_Text_OutingNeedsRank, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_OutingDone:
	msgbox Companion_Text_OutingDone, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_AskLilycove:
	msgbox Companion_Text_AskLilycove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Companion_EventScript_ReturnToMenu
	special Special_Companion_StartOuting
	msgbox Companion_Text_MeetYouThere, MSGBOX_DEFAULT
	closemessage
	warp MAP_LILYCOVE_CITY, 66, 25
	waitstate
	release
	end

Companion_EventScript_AskMossdeep:
	msgbox Companion_Text_AskMossdeep, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Companion_EventScript_ReturnToMenu
	special Special_Companion_StartOuting
	msgbox Companion_Text_MeetYouThere, MSGBOX_DEFAULT
	closemessage
	warp MAP_MOSSDEEP_CITY, 4, 30
	waitstate
	release
	end

Companion_EventScript_AskConfession:
	msgbox Companion_Text_AskConfession, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Companion_EventScript_ReturnToMenu
	special Special_Companion_StartOuting
	msgbox Companion_Text_MeetYouThere, MSGBOX_DEFAULT
	closemessage
	warp MAP_ASCENSION_SHRINE, 14, 10
	waitstate
	release
	end

Companion_EventScript_AskCeremony:
	msgbox Companion_Text_AskCeremony, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Companion_EventScript_ReturnToMenu
	special Special_Companion_StartOuting
	msgbox Companion_Text_MeetYouThere, MSGBOX_DEFAULT
	closemessage
	warp MAP_ASCENSION_SHRINE, 14, 10
	waitstate
	release
	end

@ The candidate waiting at an outing (object on Lilycove, Mossdeep and the Shrine).
Companion_EventScript_OutingPartner::
	lock
	faceplayer
	special Special_Companion_SelectOuting
	goto_if_eq VAR_0x8005, COMPANION_OUTING_CEREMONY, Companion_EventScript_Ceremony
	special Special_Companion_BufferOutingScene
	msgbox gStringVar4, MSGBOX_DEFAULT
	special Special_Companion_FinishOuting
	msgbox Companion_Text_OutingEnd, MSGBOX_DEFAULT
	closemessage
	warp MAP_CHAMPIONS_LOUNGE, 6, 5
	waitstate
	release
	end

Companion_EventScript_Ceremony:
	msgbox Companion_Text_Ceremony1, MSGBOX_DEFAULT
	special Special_Companion_FinishOuting
	playfanfare MUS_OBTAIN_BADGE
	msgbox Companion_Text_Ceremony2, MSGBOX_DEFAULT
	waitfanfare
	msgbox Companion_Text_Ceremony3, MSGBOX_DEFAULT
	closemessage
	warp MAP_CHAMPIONS_RESIDENCE, 4, 5
	waitstate
	release
	end

@ ---------------------------------------------------------------- Proposal

Companion_EventScript_Propose:
	special Special_Companion_CanPropose
	switch VAR_RESULT
	case COMPANION_PROPOSE_ALREADY, Companion_EventScript_ProposeAlready
	case COMPANION_PROPOSE_TAKEN, Companion_EventScript_ProposeTaken
	case COMPANION_PROPOSE_NOT_PARTNER, Companion_EventScript_ProposeNotYet
	case COMPANION_PROPOSE_LOW_AFFECTION, Companion_EventScript_ProposeLowAffection
	case COMPANION_PROPOSE_NO_RING, Companion_EventScript_ProposeNoRing
	msgbox Companion_Text_ProposeConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Companion_EventScript_ReturnToMenu
	special Special_Companion_Propose
	playfanfare MUS_OBTAIN_BADGE
	msgbox gStringVar4, MSGBOX_DEFAULT
	waitfanfare
	msgbox Companion_Text_Married, MSGBOX_DEFAULT
	release
	end

Companion_EventScript_ProposeAlready:
	msgbox Companion_Text_ProposeAlready, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_ProposeTaken:
	msgbox Companion_Text_ProposeTaken, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_ProposeNotYet:
	msgbox Companion_Text_ProposeNotYet, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_ProposeLowAffection:
	msgbox Companion_Text_ProposeLowAffection, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

Companion_EventScript_ProposeNoRing:
	msgbox Companion_Text_ProposeNoRing, MSGBOX_DEFAULT
	goto Companion_EventScript_ReturnToMenu

@ ---------------------------------------------------------------- Residence

Companion_EventScript_Spouse::
	lock
	faceplayer
	special Special_Companion_SelectSpouse
	setvar VAR_0x800A, 1
Companion_EventScript_SpouseMenu:
	message Companion_Text_SpouseWhatToDo
	waitmessage
	dynmultichoice 0, 0, FALSE, 6, 0, DYN_MULTICHOICE_CB_NONE, Companion_Text_OptionTalk, Companion_Text_OptionGift, Companion_Text_OptionRest, Companion_Text_OptionWalk, Companion_Text_OptionGoOut, Companion_Text_OptionLeave
	switch VAR_RESULT
	case 0, Companion_EventScript_Talk
	case 1, Companion_EventScript_Gift
	case 2, Companion_EventScript_Rest
	case 3, Companion_EventScript_Walk
	case 4, Companion_EventScript_GoOut
	release
	end

Companion_EventScript_Rest:
	msgbox Companion_Text_Rest, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	special HealPlayerParty
	playfanfare MUS_HEAL
	waitfanfare
	fadescreen FADE_FROM_BLACK
	msgbox Companion_Text_Rested, MSGBOX_DEFAULT
	goto Companion_EventScript_SpouseMenu

Companion_EventScript_Walk:
	msgbox Companion_Text_Walk, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_RESIDENCE_SPOUSE
	setflag FLAG_HIDE_RESIDENCE_SPOUSE
	special Special_Companion_StartFollowing
	fadescreen FADE_FROM_BLACK
	release
	end

@ Talking to the spouse while they walk behind the player.
Companion_EventScript_Follower::
	lock
	special Special_Companion_SelectSpouse
	msgbox Companion_Text_FollowerAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Companion_EventScript_FollowerStay
	special Special_Companion_StopFollowing
	msgbox Companion_Text_FollowerHome, MSGBOX_DEFAULT
	release
	end

Companion_EventScript_FollowerStay:
	msgbox Companion_Text_FollowerStay, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------- Map setup

Companion_OnTransition::
	special Special_Companion_SetupMap
	end

@ ---------------------------------------------------------------- Text

Companion_Text_AttendantNotChampion:
	.string "The Champions' Lounge is for\n"
	.string "Champions only. My apologies.$"

Companion_Text_AttendantWelcome:
	.string "Welcome, Champion. How may I\n"
	.string "help you?$"

Companion_Text_OptionLounge:
	.string "The Lounge$"

Companion_Text_OptionResidence:
	.string "The Residence$"

Companion_Text_OptionRing:
	.string "Promise Ring$"

Companion_Text_OptionNothing:
	.string "Nothing$"

Companion_Text_AttendantBye:
	.string "Very good, Champion.$"

Companion_Text_AttendantLounge:
	.string "Right this way. Other Champions\n"
	.string "and Leaders like to relax there.$"

Companion_Text_AttendantResidence:
	.string "Welcome home, Champion.$"

Companion_Text_AttendantNoResidence:
	.string "The Residence is for Champions\n"
	.string "who have married. Perhaps one day.$"

Companion_Text_AttendantRing:
	.string "A Promise Ring. For the one you\n"
	.string "wish to spend forever with.$"

Companion_Text_WhatToDo:
	.string "{STR_VAR_1} is here.$"

Companion_Text_SpouseWhatToDo:
	.string "{STR_VAR_1} smiles at you.$"

Companion_Text_OptionTalk:
	.string "Talk$"

Companion_Text_OptionGift:
	.string "Give a gift$"

Companion_Text_OptionBattle:
	.string "Battle$"

Companion_Text_OptionGoOut:
	.string "Go out$"

Companion_Text_OptionPropose:
	.string "Propose$"

Companion_Text_OptionRest:
	.string "Rest together$"

Companion_Text_OptionWalk:
	.string "Walk together$"

Companion_Text_OptionLeave:
	.string "Leave$"

Companion_Text_GiftConfirm:
	.string "Give the {STR_VAR_2} to\n"
	.string "{STR_VAR_1}?$"

Companion_Text_GiftDelighted:
	.string "{STR_VAR_1} is delighted!$"

Companion_Text_GiftLiked:
	.string "{STR_VAR_1} looks happy.$"

Companion_Text_GiftPolite:
	.string "{STR_VAR_1} thanks you politely.$"

Companion_Text_GiftAlready:
	.string "You already gave {STR_VAR_1}\n"
	.string "something today.$"

Companion_Text_GiftNothing:
	.string "You don't have anything that would\n"
	.string "make a good gift.$"

Companion_Text_BattleAccept:
	.string "{STR_VAR_1}: A battle? Gladly.$"

Companion_Text_BattleTomorrow:
	.string "{STR_VAR_1}: Once a day is plenty.\n"
	.string "Tomorrow?$"

Companion_Text_BattleDefeat:
	.string "That was wonderful.$"

Companion_Text_BattleAfter:
	.string "{STR_VAR_1}: Thank you. I needed\n"
	.string "that.$"

Companion_Text_OutingNotReady:
	.string "{STR_VAR_1}: Maybe another time.\n"
	.string "Let's just talk for now.$"

Companion_Text_OutingNeedsRing:
	.string "{STR_VAR_1}: ...Is there something\n"
	.string "you wanted to ask me?$"

Companion_Text_OutingNeedsRank:
	.string "{STR_VAR_1}: The summit of the\n"
	.string "Shrine. When you reach Rank V,\l"
	.string "we'll climb it together.$"

Companion_Text_OutingDone:
	.string "{STR_VAR_1}: I'm happy just being\n"
	.string "here with you.$"

Companion_Text_AskLilycove:
	.string "{STR_VAR_1}: Would you walk with me\n"
	.string "on the beach at Lilycove?$"

Companion_Text_AskMossdeep:
	.string "{STR_VAR_1}: The stars are supposed\n"
	.string "to be beautiful in Mossdeep\l"
	.string "tonight. Come with me?$"

Companion_Text_AskConfession:
	.string "{STR_VAR_1}: There's something I want\n"
	.string "to tell you. At the Shrine.\l"
	.string "Will you meet me there?$"

Companion_Text_AskCeremony:
	.string "{STR_VAR_1}: It's time, isn't it?\n"
	.string "The summit. Together.$"

Companion_Text_MeetYouThere:
	.string "{STR_VAR_1}: I'll meet you there.$"

Companion_Text_OutingEnd:
	.string "{STR_VAR_1}: Let's head back.\n"
	.string "...Thank you for today.$"

Companion_Text_Ceremony1:
	.string "{STR_VAR_1} takes your hand at the\n"
	.string "summit of the Shrine.\p"
	.string "The Oracle watches in silence.\n"
	.string "Light pours down from above.$"

Companion_Text_Ceremony2:
	.string "{STR_VAR_1} has ascended!\p"
	.string "{STR_VAR_1} is now your\n"
	.string "{STR_VAR_2}.$"

Companion_Text_Ceremony3:
	.string "{STR_VAR_1}: Forever, then.\n"
	.string "Let's go home.$"

Companion_Text_ProposeConfirm:
	.string "Offer the Promise Ring to\n"
	.string "{STR_VAR_1}?$"

Companion_Text_Married:
	.string "You and {STR_VAR_1} are married!\p"
	.string "{STR_VAR_1} moves into the Champions'\n"
	.string "Residence. The attendant in the\l"
	.string "League lobby will show you the way.$"

Companion_Text_ProposeAlready:
	.string "{STR_VAR_1}: We're already married,\n"
	.string "silly.$"

Companion_Text_ProposeTaken:
	.string "Your heart already belongs to\n"
	.string "someone.$"

Companion_Text_ProposeNotYet:
	.string "It doesn't feel like the right\n"
	.string "time yet.$"

Companion_Text_ProposeLowAffection:
	.string "{STR_VAR_1} seems to want a little\n"
	.string "more time together first.$"

Companion_Text_ProposeNoRing:
	.string "You need a Promise Ring. The\n"
	.string "attendant in the League lobby\l"
	.string "sells them.$"

Companion_Text_Rest:
	.string "{STR_VAR_1}: Stay a while. Rest.$"

Companion_Text_Rested:
	.string "You rested together. Your Pokémon\n"
	.string "are fully healed.$"

Companion_Text_Walk:
	.string "{STR_VAR_1}: Let's go together.\n"
	.string "Anywhere you like.$"

Companion_Text_FollowerAsk:
	.string "{STR_VAR_1}: Should I head home?$"

Companion_Text_FollowerHome:
	.string "{STR_VAR_1}: See you at home.$"

Companion_Text_FollowerStay:
	.string "{STR_VAR_1}: Then I'm right\n"
	.string "behind you.$"
