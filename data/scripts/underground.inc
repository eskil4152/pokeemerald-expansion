@ Underground economy. See claude-guide/features/FEATURE_UNDERGROUND_ECONOMY.md
@ Two demo NPCs stand in Littleroot Town: SHADY (the fence) and the BOSS (worker broker).
@ Currency name UCOIN is a placeholder; change it in the text below.

@ ---------------------------------------------------------------- shared

@ Sets VAR_RESULT = TRUE and prints a refusal if heat is too high.
Underground_EventScript_CheckLockout::
	special Special_Underground_IsLockedOut
	goto_if_eq VAR_RESULT, FALSE, Underground_EventScript_CheckLockout_Return
	msgbox Underground_Text_TooHot, MSGBOX_DEFAULT
Underground_EventScript_CheckLockout_Return:
	return

Underground_EventScript_ShowBalance::
	special Special_Underground_GetHeat
	buffernumberstring STR_VAR_2, VAR_RESULT
	special Special_Underground_BufferUcoins
	msgbox Underground_Text_Balance, MSGBOX_DEFAULT
	return

@ ---------------------------------------------------------------- fence

Underground_EventScript_Fence::
	lock
	faceplayer
	call Underground_EventScript_CheckLockout
	goto_if_eq VAR_RESULT, TRUE, Underground_EventScript_Leave
	msgbox Underground_Text_FenceIntro, MSGBOX_DEFAULT
	call_if_set FLAG_UNDERGROUND_TRUSTED, Arena_EventScript_FenceOfferBuyIn
Underground_EventScript_FenceMenu:
	goto_if_set FLAG_ARENA_ACCESS, Underground_EventScript_FenceMenuArena
	dynmultichoice 0, 0, TRUE, 4, 0, DYN_MULTICHOICE_CB_NONE, Underground_Text_OptSell, Underground_Text_OptBuy, Underground_Text_OptBalance, Underground_Text_OptLeave
	switch VAR_RESULT
	case 0, Underground_EventScript_Sell
	case 1, Underground_EventScript_Buy
	case 2, Underground_EventScript_FenceBalance
	goto Underground_EventScript_Leave

Underground_EventScript_FenceMenuArena:
	dynmultichoice 0, 0, TRUE, 5, 0, DYN_MULTICHOICE_CB_NONE, Underground_Text_OptSell, Underground_Text_OptBuy, Underground_Text_OptBalance, Underground_Text_OptDownstairs, Underground_Text_OptLeave
	switch VAR_RESULT
	case 0, Underground_EventScript_Sell
	case 1, Underground_EventScript_Buy
	case 2, Underground_EventScript_FenceBalance
	case 3, Arena_EventScript_GoDownstairs
	goto Underground_EventScript_Leave

Underground_EventScript_FenceBalance:
	call Underground_EventScript_ShowBalance
	goto Underground_EventScript_FenceMenu

Underground_EventScript_Sell:
	msgbox Underground_Text_WhichToSell, MSGBOX_DEFAULT
	special ChoosePartyMon
	special Special_Underground_GetMonSellValue
	goto_if_eq VAR_RESULT, 0, Underground_EventScript_CantSell
	msgbox Underground_Text_SellOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, Underground_EventScript_FenceMenu
	special Special_Underground_SellPartyMon
	setflag FLAG_UNDERGROUND_TRUSTED
	playfanfare MUS_OBTAIN_ITEM
	msgbox Underground_Text_Sold, MSGBOX_DEFAULT
	waitfanfare
	call Arena_EventScript_FenceOfferBuyIn
	goto Underground_EventScript_FenceMenu

Underground_EventScript_CantSell:
	goto_if_ge VAR_0x8004, PARTY_SIZE, Underground_EventScript_FenceMenu
	msgbox Underground_Text_CantSellThat, MSGBOX_DEFAULT
	goto Underground_EventScript_FenceMenu

Underground_EventScript_Buy:
	special Special_Underground_BufferUcoins
	msgbox Underground_Text_BuyIntro, MSGBOX_DEFAULT
	dynmultichoice 0, 0, TRUE, 4, 0, DYN_MULTICHOICE_CB_NONE, Underground_Text_OfferDratini, Underground_Text_OfferBeldum, Underground_Text_OfferLarvitar, Underground_Text_OptLeave
	switch VAR_RESULT
	case 0, Underground_EventScript_BuyDratini
	case 1, Underground_EventScript_BuyBeldum
	case 2, Underground_EventScript_BuyLarvitar
	goto Underground_EventScript_FenceMenu

Underground_EventScript_BuyDratini:
	setvar VAR_0x8004, SPECIES_DRATINI
	setvar VAR_0x8005, 20
	setvar VAR_0x8006, 500
	goto Underground_EventScript_DoBuy
Underground_EventScript_BuyBeldum:
	setvar VAR_0x8004, SPECIES_BELDUM
	setvar VAR_0x8005, 20
	setvar VAR_0x8006, 800
	goto Underground_EventScript_DoBuy
Underground_EventScript_BuyLarvitar:
	setvar VAR_0x8004, SPECIES_LARVITAR
	setvar VAR_0x8005, 20
	setvar VAR_0x8006, 800
	goto Underground_EventScript_DoBuy

Underground_EventScript_DoBuy:
	special Special_Underground_BuyMon
	goto_if_eq VAR_RESULT, UNDERGROUND_CANT_AFFORD, Underground_EventScript_CantAfford
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, Underground_EventScript_NoRoom
	setflag FLAG_UNDERGROUND_TRUSTED
	playfanfare MUS_OBTAIN_ITEM
	msgbox Underground_Text_Bought, MSGBOX_DEFAULT
	waitfanfare
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, Underground_EventScript_BoughtToPC
	goto Underground_EventScript_FenceMenu

Underground_EventScript_BoughtToPC:
	msgbox Underground_Text_SentToPC, MSGBOX_DEFAULT
	goto Underground_EventScript_FenceMenu

Underground_EventScript_CantAfford:
	msgbox Underground_Text_CantAfford, MSGBOX_DEFAULT
	goto Underground_EventScript_FenceMenu

Underground_EventScript_NoRoom:
	msgbox Underground_Text_NoRoom, MSGBOX_DEFAULT
	goto Underground_EventScript_FenceMenu

Underground_EventScript_Leave:
	msgbox Underground_Text_Leave, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------- broker

Underground_EventScript_Broker::
	lock
	faceplayer
	call Underground_EventScript_CheckLockout
	goto_if_eq VAR_RESULT, TRUE, Underground_EventScript_Leave
	msgbox Underground_Text_BrokerIntro, MSGBOX_DEFAULT
Underground_EventScript_BrokerMenu:
	dynmultichoice 0, 0, TRUE, 4, 0, DYN_MULTICHOICE_CB_NONE, Underground_Text_OptHire, Underground_Text_OptCollect, Underground_Text_OptBalance, Underground_Text_OptLeave
	switch VAR_RESULT
	case 0, Underground_EventScript_Hire
	case 1, Underground_EventScript_Collect
	case 2, Underground_EventScript_BrokerBalance
	goto Underground_EventScript_Leave

Underground_EventScript_BrokerBalance:
	call Underground_EventScript_ShowBalance
	goto Underground_EventScript_BrokerMenu

Underground_EventScript_Hire:
	dynmultichoice 0, 0, TRUE, 4, 0, DYN_MULTICHOICE_CB_NONE, Underground_Text_WorkerPickpocket, Underground_Text_WorkerSmuggler, Underground_Text_WorkerFence, Underground_Text_OptLeave
	goto_if_ge VAR_RESULT, NUM_WORKER_TYPES, Underground_EventScript_BrokerMenu
	copyvar VAR_0x8004, VAR_RESULT
	copyvar VAR_0x8007, VAR_RESULT
	special Special_Underground_BufferWorkerInfo
	buffernumberstring STR_VAR_2, VAR_RESULT
	msgbox Underground_Text_WorkerOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, Underground_EventScript_BrokerMenu
	copyvar VAR_0x8004, VAR_0x8007
	special Special_Underground_BuyWorker
	goto_if_eq VAR_RESULT, FALSE, Underground_EventScript_CantAffordWorker
	copyvar VAR_0x8004, VAR_0x8007
	special Special_Underground_BufferWorkerInfo
	msgbox Underground_Text_WorkerHired, MSGBOX_DEFAULT
	goto Underground_EventScript_BrokerMenu

Underground_EventScript_CantAffordWorker:
	msgbox Underground_Text_CantAfford, MSGBOX_DEFAULT
	goto Underground_EventScript_BrokerMenu

Underground_EventScript_Collect:
	special Special_Underground_BufferPendingPayout
	goto_if_eq VAR_RESULT, 0, Underground_EventScript_NothingToCollect
	special Special_Underground_CollectPayout
	playfanfare MUS_OBTAIN_ITEM
	msgbox Underground_Text_Collected, MSGBOX_DEFAULT
	waitfanfare
	goto Underground_EventScript_BrokerMenu

Underground_EventScript_NothingToCollect:
	msgbox Underground_Text_NothingToCollect, MSGBOX_DEFAULT
	goto Underground_EventScript_BrokerMenu

@ ---------------------------------------------------------------- text

Underground_Text_TooHot:
	.string "Not now. Too many eyes on us.\p"
	.string "Come back in a few days.$"
Underground_Text_Balance:
	.string "You hold {STR_VAR_1} UCOIN.\n"
	.string "Heat on you: {STR_VAR_2} out of 100.$"
Underground_Text_OptSell:
	.string "Sell a Pokémon$"
Underground_Text_OptBuy:
	.string "Buy a Pokémon$"
Underground_Text_OptBalance:
	.string "Check balance$"
Underground_Text_OptDownstairs:
	.string "Go downstairs$"
Underground_Text_OptLeave:
	.string "Leave$"
Underground_Text_OptHire:
	.string "Hire a worker$"
Underground_Text_OptCollect:
	.string "Collect earnings$"

Underground_Text_FenceIntro:
	.string "Psst. I move Pokémon, no questions.\n"
	.string "Buying or selling?$"
Underground_Text_WhichToSell:
	.string "Which one are you offloading?$"
Underground_Text_SellOffer:
	.string "{STR_VAR_1}… I can do {STR_VAR_2} UCOIN.\p"
	.string "Deal?$"
Underground_Text_Sold:
	.string "{STR_VAR_2} UCOIN. Pleasure doing business.$"
Underground_Text_CantSellThat:
	.string "No eggs, and I'm not taking\n"
	.string "your last one. Bad for business.$"
Underground_Text_BuyIntro:
	.string "You've got {STR_VAR_1} UCOIN.\n"
	.string "Here's what fell off the truck:$"
Underground_Text_OfferDratini:
	.string "Dratini Lv20  500u$"
Underground_Text_OfferBeldum:
	.string "Beldum Lv20  800u$"
Underground_Text_OfferLarvitar:
	.string "Larvitar Lv20  800u$"
Underground_Text_Bought:
	.string "{PLAYER} obtained a {STR_VAR_1}!$"
Underground_Text_SentToPC:
	.string "It was sent to the PC.$"
Underground_Text_CantAfford:
	.string "You can't cover that.$"
Underground_Text_NoRoom:
	.string "You've got nowhere to put it.$"
Underground_Text_Leave:
	.string "You were never here.$"

Underground_Text_BrokerIntro:
	.string "People work for me. They could\n"
	.string "work for you, for a price.$"
Underground_Text_WorkerPickpocket:
	.string "Pickpocket  200u$"
Underground_Text_WorkerSmuggler:
	.string "Smuggler  600u$"
Underground_Text_WorkerFence:
	.string "Fence  1500u$"
Underground_Text_WorkerOffer:
	.string "A {STR_VAR_1} brings in {STR_VAR_2} UCOIN\n"
	.string "a day. You own {STR_VAR_3}.\p"
	.string "Hire one?$"
Underground_Text_WorkerHired:
	.string "Done. You now own {STR_VAR_3}.\p"
	.string "Collect from me any day after today.$"
Underground_Text_Collected:
	.string "Your cut: {STR_VAR_1} UCOIN.$"
Underground_Text_NothingToCollect:
	.string "Nothing yet. Your people earn\n"
	.string "{STR_VAR_2} UCOIN a day. Come back tomorrow.$"
