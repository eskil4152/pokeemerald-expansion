@ Wager battles. See claude-guide/features/FEATURE_WAGER_BATTLES.md
@
@ Use from any NPC script (at top level, not inside a called subroutine,
@ because wagerbattle rebuilds the script context):
@
@   setvar VAR_WAGER_OPP_MONEY, 3000
@   setvar VAR_WAGER_OPP_SPECIES, SPECIES_ABSOL      @ or SPECIES_NONE
@   setvar VAR_WAGER_OPP_LEVEL, 20
@   call Wager_EventScript_ChooseStake                @ VAR_RESULT = TRUE if a stake was placed
@   goto_if_eq VAR_RESULT, FALSE, MyScript_Declined
@   wagerbattle TRAINER_X, MyScript_Text_TrainerLost
@   call Wager_EventScript_Settle                     @ pays out or collects, prints the result

@ Player picks what to stake. Sets VAR_RESULT = TRUE if a stake was placed.
Wager_EventScript_ChooseStake::
	special Wager_ClearPlayerStake
	dynmultichoice 0, 0, TRUE, 4, 0, DYN_MULTICHOICE_CB_NONE, Wager_Text_OptionMoney, Wager_Text_OptionPokemon, Wager_Text_OptionBoth, Wager_Text_OptionNothing
	switch VAR_RESULT
	case 0, Wager_EventScript_StakeMoney
	case 1, Wager_EventScript_StakeMon
	case 2, Wager_EventScript_StakeBoth
	goto Wager_EventScript_Declined

Wager_EventScript_StakeMoney:
	call Wager_EventScript_PickMoneyAmount
	goto_if_eq VAR_RESULT, FALSE, Wager_EventScript_Declined
	goto Wager_EventScript_Confirm

Wager_EventScript_StakeMon:
	call Wager_EventScript_PickMon
	goto_if_eq VAR_RESULT, FALSE, Wager_EventScript_Declined
	goto Wager_EventScript_Confirm

Wager_EventScript_StakeBoth:
	call Wager_EventScript_PickMoneyAmount
	goto_if_eq VAR_RESULT, FALSE, Wager_EventScript_Declined
	call Wager_EventScript_PickMon
	goto_if_eq VAR_RESULT, FALSE, Wager_EventScript_Declined
	goto Wager_EventScript_Confirm

Wager_EventScript_Confirm:
	special Wager_BufferStake
	msgbox Wager_Text_ConfirmStake, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, Wager_EventScript_Declined
	setvar VAR_RESULT, TRUE
	return

Wager_EventScript_Declined:
	special Wager_ClearPlayerStake
	setvar VAR_RESULT, FALSE
	return

@ Subroutine: pick a money amount. VAR_RESULT = TRUE if set.
Wager_EventScript_PickMoneyAmount:
	showmoneybox 0, 0
	dynmultichoice 0, 4, TRUE, 5, 0, DYN_MULTICHOICE_CB_NONE, Wager_Text_Amount500, Wager_Text_Amount2000, Wager_Text_Amount5000, Wager_Text_Amount20000, Wager_Text_AmountCancel
	hidemoneybox
	switch VAR_RESULT
	case 0, Wager_EventScript_TrySetMoney500
	case 1, Wager_EventScript_TrySetMoney2000
	case 2, Wager_EventScript_TrySetMoney5000
	case 3, Wager_EventScript_TrySetMoney20000
	setvar VAR_RESULT, FALSE
	return

Wager_EventScript_TrySetMoney500:
	setvar VAR_0x8004, 500
	goto Wager_EventScript_TrySetMoney
Wager_EventScript_TrySetMoney2000:
	setvar VAR_0x8004, 2000
	goto Wager_EventScript_TrySetMoney
Wager_EventScript_TrySetMoney5000:
	setvar VAR_0x8004, 5000
	goto Wager_EventScript_TrySetMoney
Wager_EventScript_TrySetMoney20000:
	setvar VAR_0x8004, 20000
	goto Wager_EventScript_TrySetMoney

Wager_EventScript_TrySetMoney:
	special Wager_TrySetMoneyStake
	goto_if_eq VAR_RESULT, TRUE, Wager_EventScript_MoneySet
	msgbox Wager_Text_NotEnoughMoney, MSGBOX_DEFAULT
	setvar VAR_RESULT, FALSE
	return

Wager_EventScript_MoneySet:
	setvar VAR_RESULT, TRUE
	return

@ Subroutine: pick a party Pokémon. VAR_RESULT = TRUE if set.
Wager_EventScript_PickMon:
	msgbox Wager_Text_WhichPokemon, MSGBOX_DEFAULT
	special ChoosePartyMon
	special Wager_TryStakePartyMon
	goto_if_eq VAR_RESULT, WAGER_STAKE_OK, Wager_EventScript_MonSet
	goto_if_eq VAR_RESULT, WAGER_STAKE_CANCELLED, Wager_EventScript_MonCancelled
	msgbox Wager_Text_CantStakeThat, MSGBOX_DEFAULT
Wager_EventScript_MonCancelled:
	setvar VAR_RESULT, FALSE
	return

Wager_EventScript_MonSet:
	msgbox Wager_Text_MonStaked, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

@ Call right after wagerbattle. Reads VAR_RESULT (TRUE = player lost).
Wager_EventScript_Settle::
	copyvar VAR_0x8004, VAR_RESULT
	special Wager_Settle
	goto_if_eq VAR_RESULT, WAGER_RESULT_LOST, Wager_EventScript_SettleLost
	call_if_ne VAR_0x8006, 0, Wager_EventScript_PrintWonMoney
	goto_if_eq VAR_0x8005, WAGER_MON_NONE, Wager_EventScript_SettleDone
	goto_if_eq VAR_0x8005, MON_GIVEN_TO_PARTY, Wager_EventScript_WonMonParty
	goto_if_eq VAR_0x8005, MON_GIVEN_TO_PC, Wager_EventScript_WonMonPC
	msgbox Wager_Text_WonMonNoRoom, MSGBOX_DEFAULT
	return

Wager_EventScript_PrintWonMoney:
	playfanfare MUS_OBTAIN_ITEM
	msgbox Wager_Text_YouWonMoney, MSGBOX_DEFAULT
	waitfanfare
	return

Wager_EventScript_WonMonParty:
	playfanfare MUS_OBTAIN_ITEM
	msgbox Wager_Text_WonMonParty, MSGBOX_DEFAULT
	waitfanfare
	return

Wager_EventScript_WonMonPC:
	playfanfare MUS_OBTAIN_ITEM
	msgbox Wager_Text_WonMonPC, MSGBOX_DEFAULT
	waitfanfare
	return

Wager_EventScript_SettleLost:
	call_if_ne VAR_0x8006, 0, Wager_EventScript_PrintLostMoney
	goto_if_eq VAR_0x8005, WAGER_MON_TAKEN, Wager_EventScript_LostMon
Wager_EventScript_SettleDone:
	return

Wager_EventScript_PrintLostMoney:
	msgbox Wager_Text_YouLostMoney, MSGBOX_DEFAULT
	return

Wager_EventScript_LostMon:
	msgbox Wager_Text_LostMon, MSGBOX_DEFAULT
	return

@ Demo bookie in Littleroot Town (object event in data/maps/LittlerootTown/map.json).
Wager_EventScript_DemoBookie::
	lock
	faceplayer
	msgbox Wager_Text_DemoIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, Wager_EventScript_DemoDeclined
	setvar VAR_WAGER_OPP_MONEY, 3000
	setvar VAR_WAGER_OPP_SPECIES, SPECIES_ABSOL
	setvar VAR_WAGER_OPP_LEVEL, 20
	call Wager_EventScript_ChooseStake
	goto_if_eq VAR_RESULT, FALSE, Wager_EventScript_DemoDeclined
	msgbox Wager_Text_DemoLetsGo, MSGBOX_DEFAULT
	closemessage
	wagerbattle TRAINER_WAGER_DEMO, Wager_Text_DemoLost, Wager_Text_DemoWon
	call Wager_EventScript_Settle
	msgbox Wager_Text_DemoComeBack, MSGBOX_DEFAULT
	release
	end

Wager_EventScript_DemoDeclined:
	msgbox Wager_Text_DemoDeclined, MSGBOX_DEFAULT
	release
	end

Wager_Text_OptionMoney:
	.string "Money$"
Wager_Text_OptionPokemon:
	.string "Pokémon$"
Wager_Text_OptionBoth:
	.string "Both$"
Wager_Text_OptionNothing:
	.string "Nothing$"

Wager_Text_Amount500:
	.string "¥500$"
Wager_Text_Amount2000:
	.string "¥2000$"
Wager_Text_Amount5000:
	.string "¥5000$"
Wager_Text_Amount20000:
	.string "¥20000$"
Wager_Text_AmountCancel:
	.string "Cancel$"

Wager_Text_NotEnoughMoney:
	.string "You don't have that much.$"
Wager_Text_WhichPokemon:
	.string "Which Pokémon will you put up?$"
Wager_Text_CantStakeThat:
	.string "You can't stake that one.\p"
	.string "Eggs and your last Pokémon\n"
	.string "are off the table.$"
Wager_Text_MonStaked:
	.string "{STR_VAR_1} is on the line.$"
Wager_Text_ConfirmStake:
	.string "Your stake: ¥{STR_VAR_1} and {STR_VAR_2}.\p"
	.string "Deal?$"

Wager_Text_YouWonMoney:
	.string "{PLAYER} won ¥{STR_VAR_1}!$"
Wager_Text_WonMonParty:
	.string "{PLAYER} won the {STR_VAR_2}!\p"
	.string "It joined the party.$"
Wager_Text_WonMonPC:
	.string "{PLAYER} won the {STR_VAR_2}!\p"
	.string "It was sent to the PC.$"
Wager_Text_WonMonNoRoom:
	.string "{PLAYER} won the {STR_VAR_2},\n"
	.string "but there was no room for it…$"
Wager_Text_YouLostMoney:
	.string "{PLAYER} lost ¥{STR_VAR_1}…$"
Wager_Text_LostMon:
	.string "{STR_VAR_2} was taken away…$"

Wager_Text_DemoIntro:
	.string "Fancy a wager?\p"
	.string "I'll put up ¥3000 and my Absol.\n"
	.string "What will you risk?$"
Wager_Text_DemoLetsGo:
	.string "Then let's see what you've got!$"
Wager_Text_DemoLost:
	.string "Ha! Beaten fair and square.$"
Wager_Text_DemoWon:
	.string "The house always wins.$"
Wager_Text_DemoComeBack:
	.string "Come back whenever you feel lucky.$"
Wager_Text_DemoDeclined:
	.string "Suit yourself.$"
