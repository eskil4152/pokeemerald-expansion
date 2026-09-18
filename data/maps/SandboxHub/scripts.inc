@ Sandbox hub. See claude-guide/features/FEATURE_ARENA_AND_HUB.md
@ Reached through the guide in Littleroot Town; the exit mat returns to where
@ the guide stored the dynamic warp.

SandboxHub_MapScripts::
	.byte 0

@ Guide standing in Littleroot Town (object event in LittlerootTown/map.json).
SandboxHub_EventScript_Guide::
	lock
	faceplayer
	msgbox SandboxHub_Text_GuideOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, SandboxHub_EventScript_GuideDeclined
	closemessage
	setdynamicwarp MAP_LITTLEROOT_TOWN, 18, 13
	warp MAP_SANDBOX_HUB, 0
	waitstate
	release
	end

SandboxHub_EventScript_GuideDeclined:
	msgbox SandboxHub_Text_GuideDeclined, MSGBOX_DEFAULT
	release
	end

SandboxHub_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_SANDBOX_HUB_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

SandboxHub_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart SandboxHub_Mart_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

SandboxHub_EventScript_Relearner::
	goto Common_EventScript_MoveRelearner
	end

	.align 2
SandboxHub_Mart_Items:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_QUICK_BALL
	.2byte ITEM_DUSK_BALL
	.2byte ITEM_TIMER_BALL
	.2byte ITEM_HEAL_BALL
	.2byte ITEM_LUXURY_BALL
	.2byte ITEM_MASTER_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ETHER
	.2byte ITEM_MAX_ETHER
	.2byte ITEM_ELIXIR
	.2byte ITEM_MAX_ELIXIR
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_RARE_CANDY
	.2byte ITEM_HP_UP
	.2byte ITEM_PROTEIN
	.2byte ITEM_IRON
	.2byte ITEM_CALCIUM
	.2byte ITEM_ZINC
	.2byte ITEM_CARBOS
	.2byte ITEM_PP_UP
	.2byte ITEM_PP_MAX
	.2byte ITEM_ABILITY_CAPSULE
	.2byte ITEM_ABILITY_PATCH
	.2byte ITEM_BOTTLE_CAP
	.2byte ITEM_GOLD_BOTTLE_CAP
	.2byte ITEM_FIRE_STONE
	.2byte ITEM_WATER_STONE
	.2byte ITEM_THUNDER_STONE
	.2byte ITEM_LEAF_STONE
	.2byte ITEM_MOON_STONE
	.2byte ITEM_SUN_STONE
	.2byte ITEM_SHINY_STONE
	.2byte ITEM_DUSK_STONE
	.2byte ITEM_DAWN_STONE
	.2byte ITEM_ICE_STONE
	.2byte ITEM_LEFTOVERS
	.2byte ITEM_LIFE_ORB
	.2byte ITEM_CHOICE_BAND
	.2byte ITEM_CHOICE_SPECS
	.2byte ITEM_CHOICE_SCARF
	.2byte ITEM_FOCUS_SASH
	.2byte ITEM_ASSAULT_VEST
	.2byte ITEM_ROCKY_HELMET
	.2byte ITEM_EVIOLITE
	.2byte ITEM_LUCKY_EGG
	.2byte ITEM_EXP_SHARE
	pokemartlistend

SandboxHub_Text_GuideOffer:
	.string "Need a place to rest, shop and\n"
	.string "rethink your moves?\p"
	.string "I can take you to the hub.$"
SandboxHub_Text_GuideDeclined:
	.string "Any time.$"
