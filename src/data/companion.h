// Companion tuning, candidates and lines. Included by src/companion.c.
// Change values freely; the code only reads them.

// Affection needed to reach each stage. Also the floor affection can drift
// down to once that stage is reached.
static const u16 sStageAffection[COMPANION_STAGE_COUNT] =
{
    [COMPANION_STAGE_NONE]     = 0,
    [COMPANION_STAGE_MET]      = 0,
    [COMPANION_STAGE_FRIEND]   = 150,
    [COMPANION_STAGE_CLOSE]    = 350,
    [COMPANION_STAGE_PARTNER]  = 600,
    [COMPANION_STAGE_SPOUSE]   = 700,   // they accept the ring from here
    [COMPANION_STAGE_ASCENDED] = 850,   // plus Rank V
};

// The outing that leads out of each stage. SPOUSE -> ASCENDED is the ceremony.
static const u8 sStageOuting[COMPANION_STAGE_COUNT] =
{
    [COMPANION_STAGE_MET]     = COMPANION_OUTING_LILYCOVE,
    [COMPANION_STAGE_FRIEND]  = COMPANION_OUTING_MOSSDEEP,
    [COMPANION_STAGE_CLOSE]   = COMPANION_OUTING_CONFESSION,
    [COMPANION_STAGE_SPOUSE]  = COMPANION_OUTING_CEREMONY,
};

// Gift items anyone accepts politely (COMPANION_NEUTRAL_GIFT), after likes.
static const u16 sNeutralGifts[] =
{
    ITEM_LAVA_COOKIE, ITEM_OLD_GATEAU, ITEM_RAGE_CANDY_BAR, ITEM_CASTELIACONE,
    ITEM_LUMIOSE_GALETTE, ITEM_SHALOUR_SABLE, ITEM_BIG_MALASADA, ITEM_SWEET_HEART,
    ITEM_POKE_DOLL, ITEM_PRETTY_FEATHER, ITEM_BIG_PEARL, ITEM_STAR_PIECE,
};

// A liked gift worth at least this much gets the delighted reaction.
#define GIFT_DELIGHT_THRESHOLD  30

// ------------------------------------------------------------------ Likes

static const struct CompanionGift sLikes_Cynthia[] =
{
    { ITEM_COMET_SHARD, 60 }, { ITEM_RELIC_CROWN, 55 }, { ITEM_ODD_KEYSTONE, 45 },
    { ITEM_HELIX_FOSSIL, 40 }, { ITEM_DRAGON_SCALE, 35 }, { ITEM_DRAGON_FANG, 35 },
    { ITEM_OLD_GATEAU, 30 }, { ITEM_RELIC_STATUE, 30 },
};
static const struct CompanionGift sLikes_Roxanne[] =
{
    { ITEM_RARE_BONE, 45 }, { ITEM_HELIX_FOSSIL, 40 }, { ITEM_HARD_STONE, 30 },
    { ITEM_STAR_PIECE, 25 }, { ITEM_LAVA_COOKIE, 15 },
};
static const struct CompanionGift sLikes_Flannery[] =
{
    { ITEM_LAVA_COOKIE, 40 }, { ITEM_CHARCOAL, 30 }, { ITEM_RAGE_CANDY_BAR, 20 },
    { ITEM_BIG_MALASADA, 20 },
};
static const struct CompanionGift sLikes_Winona[] =
{
    { ITEM_PRETTY_FEATHER, 40 }, { ITEM_SHARP_BEAK, 30 }, { ITEM_SWEET_HEART, 20 },
    { ITEM_BIG_PEARL, 15 },
};
static const struct CompanionGift sLikes_Phoebe[] =
{
    { ITEM_REAPER_CLOTH, 40 }, { ITEM_SPELL_TAG, 30 }, { ITEM_OLD_GATEAU, 25 },
    { ITEM_POKE_DOLL, 20 },
};
static const struct CompanionGift sLikes_Glacia[] =
{
    { ITEM_PEARL_STRING, 40 }, { ITEM_NEVER_MELT_ICE, 30 }, { ITEM_CASTELIACONE, 25 },
    { ITEM_BIG_PEARL, 25 },
};
static const struct CompanionGift sLikes_Steven[] =
{
    { ITEM_COMET_SHARD, 50 }, { ITEM_STAR_PIECE, 35 }, { ITEM_RARE_BONE, 30 },
    { ITEM_METAL_COAT, 25 },
};
static const struct CompanionGift sLikes_Wallace[] =
{
    { ITEM_PEARL_STRING, 45 }, { ITEM_PRISM_SCALE, 40 }, { ITEM_BIG_PEARL, 30 },
    { ITEM_MYSTIC_WATER, 25 },
};
static const struct CompanionGift sLikes_Brawly[] =
{
    { ITEM_BIG_MALASADA, 35 }, { ITEM_BLACK_BELT, 30 }, { ITEM_RAGE_CANDY_BAR, 20 },
    { ITEM_LAVA_COOKIE, 15 },
};
static const struct CompanionGift sLikes_Sidney[] =
{
    { ITEM_BLACK_GLASSES, 35 }, { ITEM_RAGE_CANDY_BAR, 25 }, { ITEM_BIG_MALASADA, 20 },
    { ITEM_STAR_PIECE, 15 },
};
static const struct CompanionGift sLikes_Lance[] =
{
    { ITEM_DRAGON_SCALE, 45 }, { ITEM_DRAGON_FANG, 35 }, { ITEM_COMET_SHARD, 30 },
    { ITEM_RAGE_CANDY_BAR, 25 },
};

#define LIKES(list) .likes = list, .likeCount = ARRAY_COUNT(list)

// ------------------------------------------------------------------ Candidates
//
// lines[stage]: up to three things they say, picked at random. NULL = unused.
// outings: the Lilycove, Mossdeep and Shrine confession scenes.

const struct CompanionInfo gCompanions[COMPANION_COUNT] =
{
    [COMPANION_CYNTHIA] =
    {
        .name = COMPOUND_STRING("CYNTHIA"),
        .graphicsId = OBJ_EVENT_GFX_COOLTRAINER_F,  // placeholder until real sprites exist
        .trainerId = TRAINER_VISITOR_CYNTHIA,
        .partnerId = PARTNER_CYNTHIA,
        .gender = FEMALE,
        .unlock = COMPANION_UNLOCK_VISITOR,
        .visitorId = ASCENSION_VISITOR_CYNTHIA,
        LIKES(sLikes_Cynthia),
        .lines =
        {
            [COMPANION_STAGE_MET] = {
                COMPOUND_STRING("CYNTHIA: Oh, it's you. I was reading\nabout Hoenn's old legends.\pThey're different from Sinnoh's.\nWarmer, somehow."),
                COMPOUND_STRING("CYNTHIA: I came for a week and\nsomehow I'm still here.\pI suppose I know why."),
                COMPOUND_STRING("CYNTHIA: A Champion's job is lonely.\nIt's nice to have someone who\lunderstands."),
            },
            [COMPANION_STAGE_FRIEND] = {
                COMPOUND_STRING("CYNTHIA: {PLAYER}! I saved you\nsome tea. It's gone cold, but\lI saved it."),
                COMPOUND_STRING("CYNTHIA: I keep thinking about\nthe sea at Lilycove."),
                COMPOUND_STRING("CYNTHIA: Garchomp missed you.\nDon't tell it I said that."),
            },
            [COMPANION_STAGE_CLOSE] = {
                COMPOUND_STRING("CYNTHIA: There you are. The lounge\nis so quiet without you."),
                COMPOUND_STRING("CYNTHIA: I told the Sinnoh League\nI'm staying longer.\pThey didn't ask why. Good."),
                COMPOUND_STRING("CYNTHIA: I still see those stars\nwhen I close my eyes."),
            },
            [COMPANION_STAGE_PARTNER] = {
                COMPOUND_STRING("CYNTHIA: {PLAYER}. I missed you.\nCome here."),
                COMPOUND_STRING("CYNTHIA: Everywhere I look in Hoenn\nnow, I think of you."),
                COMPOUND_STRING("CYNTHIA: I love you. I'll never\nget tired of saying it."),
            },
            [COMPANION_STAGE_SPOUSE] = {
                COMPOUND_STRING("CYNTHIA: Welcome home, my love."),
                COMPOUND_STRING("CYNTHIA: Sinnoh can wait.\nThis is home now."),
                COMPOUND_STRING("CYNTHIA: The summit of the Shrine\nis waiting for us, isn't it?"),
            },
            [COMPANION_STAGE_ASCENDED] = {
                COMPOUND_STRING("CYNTHIA: They call me a Goddess\nnow. I only answer to you."),
                COMPOUND_STRING("CYNTHIA: Every legend I ever studied\nled me here. To you."),
                COMPOUND_STRING("CYNTHIA: I'm yours. I always\nwill be."),
            },
        },
        .outings =
        {
            COMPOUND_STRING("CYNTHIA: The sea here is so blue.\pIn Sinnoh the water is colder.\nYou learn to love it anyway.\p...Thank you for coming. I don't\nget to just walk very often."),
            COMPOUND_STRING("CYNTHIA: Look. You can see every\nstar from here.\pThe old myths say Arceus shaped\nthe world from nothing.\pI used to think that was the most\nbeautiful story I knew.\p...I'm not so sure anymore."),
            COMPOUND_STRING("CYNTHIA: {PLAYER}. I've studied\nlegends my whole life.\pI never thought I'd find one\nof my own.\pI love you. I think I have for\na long time."),
        },
        .proposal = COMPOUND_STRING("CYNTHIA: ...Yes. Yes, of course.\pI crossed an ocean to find a\nworthy rival. I found you instead."),
        .whatCouldHaveBeen = COMPOUND_STRING("CYNTHIA: I'm happy for you.\nTruly.\p...Sometimes I wonder, though."),
    },
    [COMPANION_ROXANNE] =
    {
        .name = COMPOUND_STRING("ROXANNE"),
        .graphicsId = OBJ_EVENT_GFX_ROXANNE,
        .trainerId = TRAINER_ROXANNE_5,
        .partnerId = PARTNER_ROXANNE,
        .gender = FEMALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Roxanne),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("ROXANNE: My first challenger became\nChampion. I tell my students\lall about you."), COMPOUND_STRING("ROXANNE: I'm grading papers.\nYou can sit, if you're quiet.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("ROXANNE: I read about a fossil dig\nnear Rustboro. Interested?"), COMPOUND_STRING("ROXANNE: You're better company\nthan my textbooks.") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("ROXANNE: I made notes on our\nlast talk. Is that strange?") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("ROXANNE: I studied everything\nexcept how to stop thinking\labout you.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("ROXANNE: Welcome home. I made\na lesson plan for our weekend.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("ROXANNE: Even stone erodes.\nWhat we have won't.") },
        },
        .outings =
        {
            COMPOUND_STRING("ROXANNE: Sand is just rock that\nhad a very long journey.\p...I'm rambling. I ramble when\nI'm nervous. Walk with me?"),
            COMPOUND_STRING("ROXANNE: Meteorites fell here once.\nPieces of other worlds.\pI feel a little like that\naround you."),
            COMPOUND_STRING("ROXANNE: I prepared a speech.\nI've forgotten all of it.\p...I love you. That was the\nwhole speech."),
        },
        .proposal = COMPOUND_STRING("ROXANNE: Yes! I mean... yes.\nI accept. Wholeheartedly."),
        .whatCouldHaveBeen = COMPOUND_STRING("ROXANNE: I'm glad you're happy.\nI'll keep studying. It helps."),
    },
    [COMPANION_FLANNERY] =
    {
        .name = COMPOUND_STRING("FLANNERY"),
        .graphicsId = OBJ_EVENT_GFX_FLANNERY,
        .trainerId = TRAINER_FLANNERY_5,
        .partnerId = PARTNER_FLANNERY,
        .gender = FEMALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Flannery),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("FLANNERY: Champion! Ha, I knew\nyou'd make it. Fight me again\lsometime, OK?"), COMPOUND_STRING("FLANNERY: This lounge is way too\nquiet. Needs more fire.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("FLANNERY: The hot springs miss you.\nOK, I miss you. Whatever!") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("FLANNERY: My face is red because\nof the heat. Obviously.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("FLANNERY: You make my heart burn\nhotter than Mt. Chimney!") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("FLANNERY: You're home! Dinner's\nspicy. Deal with it.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("FLANNERY: A flame that never goes\nout. That's us now.") },
        },
        .outings =
        {
            COMPOUND_STRING("FLANNERY: Ugh, the sea! Too much\nwater!\p...But the sunset's pretty.\nI'll give it that."),
            COMPOUND_STRING("FLANNERY: The stars look like\nembers, right?\pGrandpa says a good fire needs\ntwo logs.\pNo reason. I just remembered it."),
            COMPOUND_STRING("FLANNERY: OK! I'm just going to\nsay it! No backing down!\pI love you! There! It's out!\n...Say something!"),
        },
        .proposal = COMPOUND_STRING("FLANNERY: Yes! Oh wow, yes!\nI'm going to cry. Don't look!"),
        .whatCouldHaveBeen = COMPOUND_STRING("FLANNERY: Congrats! Really.\nI'll just... go vent on a volcano."),
    },
    [COMPANION_WINONA] =
    {
        .name = COMPOUND_STRING("WINONA"),
        .graphicsId = OBJ_EVENT_GFX_WINONA,
        .trainerId = TRAINER_WINONA_5,
        .partnerId = PARTNER_WINONA,
        .gender = FEMALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Winona),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("WINONA: Hello, Champion. Altaria\nhums whenever you're near."), COMPOUND_STRING("WINONA: I came here to rest my\nwings for a while.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("WINONA: The wind was kind today.\nIt carried me here to you.") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("WINONA: Up in the sky, I look for\nyou down below.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("WINONA: With you, I feel like I'm\nflying even on the ground.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("WINONA: Welcome home. Every bird\nreturns to its nest.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("WINONA: We soar above the clouds\nnow. Together.") },
        },
        .outings =
        {
            COMPOUND_STRING("WINONA: Wingull glide on the sea\nwind here.\pI love watching them.\nThank you for walking with me."),
            COMPOUND_STRING("WINONA: At night, flying Pokémon\nnavigate by the stars.\pI think I've found my\nnorth star."),
            COMPOUND_STRING("WINONA: I've always been free.\nI never wanted to land.\pUntil you. I love you,\n{PLAYER}."),
        },
        .proposal = COMPOUND_STRING("WINONA: Yes. Let's share the sky\nfor the rest of our lives."),
        .whatCouldHaveBeen = COMPOUND_STRING("WINONA: I wish you both fair winds.\nTruly."),
    },
    [COMPANION_PHOEBE] =
    {
        .name = COMPOUND_STRING("PHOEBE"),
        .graphicsId = OBJ_EVENT_GFX_PHOEBE,
        .trainerId = TRAINER_PHOEBE,
        .partnerId = PARTNER_PHOEBE,
        .gender = FEMALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Phoebe),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("PHOEBE: Ahahaha! The Champion!\nMy ghosts like you, you know."), COMPOUND_STRING("PHOEBE: Grandma says hi. From\nMt. Pyre. She's alive, don't worry.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("PHOEBE: Dusclops keeps staring at\nyou. That's a compliment!") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("PHOEBE: My heart's doing a spooky\nthing when you're here.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("PHOEBE: I'd haunt you forever.\nIn a nice way!") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("PHOEBE: Welcome home! The ghosts\ntidied up. Mostly.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("PHOEBE: Not even the afterlife\ncould part us now. Ahahaha!") },
        },
        .outings =
        {
            COMPOUND_STRING("PHOEBE: The beach at night is\nthe best. Things glow in the water!\pAhaha... thanks for coming.\nMost people get scared."),
            COMPOUND_STRING("PHOEBE: Grandma says stars are\nspirits watching over us.\pI hope they're watching now."),
            COMPOUND_STRING("PHOEBE: I'm not scary, am I?\nBecause I... I really like you.\pNo. I love you. Boo."),
        },
        .proposal = COMPOUND_STRING("PHOEBE: Yes! Till death do us part,\nand then some! Ahahaha!"),
        .whatCouldHaveBeen = COMPOUND_STRING("PHOEBE: Aww. Well, my ghosts will\nkeep me company. Be happy!"),
    },
    [COMPANION_GLACIA] =
    {
        .name = COMPOUND_STRING("GLACIA"),
        .graphicsId = OBJ_EVENT_GFX_GLACIA,
        .trainerId = TRAINER_GLACIA,
        .partnerId = PARTNER_GLACIA,
        .gender = FEMALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Glacia),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("GLACIA: Champion. You were the first\nin a long time to melt my ice."), COMPOUND_STRING("GLACIA: Do sit. I don't bite.\nUsually.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("GLACIA: I find myself looking\nforward to your visits.") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("GLACIA: It's warm in here.\nOr perhaps it's just you.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("GLACIA: I was ice for so long.\nYou thawed all of it.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("GLACIA: Welcome home, darling.\nTea is ready.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("GLACIA: Eternal, like the glaciers.\nAnd far warmer.") },
        },
        .outings =
        {
            COMPOUND_STRING("GLACIA: The sea breeze is cool.\nI like that.\pI'm glad I came to Hoenn.\nMore glad now."),
            COMPOUND_STRING("GLACIA: The stars are ice in the\nsky, some say.\pCold, and still, and beautiful.\nNot unlike me, I once thought."),
            COMPOUND_STRING("GLACIA: I'll be direct, as always.\pI love you. I don't say it\nlightly. I've never said it."),
        },
        .proposal = COMPOUND_STRING("GLACIA: Yes. My heart is yours,\nwholly and forever."),
        .whatCouldHaveBeen = COMPOUND_STRING("GLACIA: How lovely for you.\nI mean it... mostly."),
    },
    [COMPANION_STEVEN] =
    {
        .name = COMPOUND_STRING("STEVEN"),
        .graphicsId = OBJ_EVENT_GFX_STEVEN,
        .trainerId = TRAINER_STEVEN,
        .partnerId = PARTNER_STEVEN_COMPANION,
        .gender = MALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Steven),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("STEVEN: The new Champion. I knew\nfrom our first meeting you'd\lgo far."), COMPOUND_STRING("STEVEN: Have you seen any rare\nstones lately?") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("STEVEN: I found a stone that\nreminded me of you. Strange,\lisn't it?") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("STEVEN: I skipped a cave expedition\nto be here. Worth it.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("STEVEN: Of all the treasures I've\nfound, you're the rarest.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("STEVEN: Welcome home. I polished\nour stones. All of them.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("STEVEN: A bond harder than\ndiamond. Truly.") },
        },
        .outings =
        {
            COMPOUND_STRING("STEVEN: Walking on the beach\nwithout looking for stones...\pThis is a first. I like it."),
            COMPOUND_STRING("STEVEN: The Space Center is just\nover there. Meteorites, stars...\pAnd you. Not a bad night."),
            COMPOUND_STRING("STEVEN: I've searched every cave\nin Hoenn for something precious.\pIt was you. I love you."),
        },
        .proposal = COMPOUND_STRING("STEVEN: Yes. I'll treasure this\nmore than any stone."),
        .whatCouldHaveBeen = COMPOUND_STRING("STEVEN: I'm glad you found your\ntreasure. Truly."),
    },
    [COMPANION_WALLACE] =
    {
        .name = COMPOUND_STRING("WALLACE"),
        .graphicsId = OBJ_EVENT_GFX_WALLACE,
        .trainerId = TRAINER_WALLACE,
        .partnerId = PARTNER_WALLACE,
        .gender = MALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Wallace),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("WALLACE: My successor! You battle\nwith such elegance."), COMPOUND_STRING("WALLACE: Beauty is in how you\ntreat your Pokémon.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("WALLACE: Your presence is more\nradiant than Milotic's scales.") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("WALLACE: I find I've been dressing\nup for your visits.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("WALLACE: The most beautiful thing\nin Hoenn is standing before me.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("WALLACE: Welcome home, my dear.\nEverything is perfect now.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("WALLACE: An eternal masterpiece.\nThat is what we are.") },
        },
        .outings =
        {
            COMPOUND_STRING("WALLACE: The sea at sunset!\nWhat a stage.\pAnd you, the finest performer\nI know."),
            COMPOUND_STRING("WALLACE: Look how the stars\nreflect on the water.\pBeauty doubled. Like joy,\nwhen shared."),
            COMPOUND_STRING("WALLACE: I have given my heart\nto the water all my life.\pNow I give it to you.\nI love you."),
        },
        .proposal = COMPOUND_STRING("WALLACE: Yes! A wedding worthy of\na Contest stage! Oh, yes!"),
        .whatCouldHaveBeen = COMPOUND_STRING("WALLACE: A beautiful match.\nI shall applaud from afar."),
    },
    [COMPANION_BRAWLY] =
    {
        .name = COMPOUND_STRING("BRAWLY"),
        .graphicsId = OBJ_EVENT_GFX_BRAWLY,
        .trainerId = TRAINER_BRAWLY_5,
        .partnerId = PARTNER_BRAWLY,
        .gender = MALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Brawly),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("BRAWLY: Champ! You rode the big\nwave all the way, huh?"), COMPOUND_STRING("BRAWLY: Wanna go surfing later?") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("BRAWLY: You're one cool wave,\nyou know that?") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("BRAWLY: Wiped out three times\nthinking about you today.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("BRAWLY: Best wave of my life?\nYou. Easy.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("BRAWLY: Home! I waxed the boards.\nBoth of 'em.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("BRAWLY: Riding the endless wave\ntogether. Gnarly.") },
        },
        .outings =
        {
            COMPOUND_STRING("BRAWLY: Perfect swell today!\pBut honestly? I'd rather just\nhang here with you."),
            COMPOUND_STRING("BRAWLY: You ever look at the stars\nand feel tiny?\pWith you around I feel huge.\nIn a good way!"),
            COMPOUND_STRING("BRAWLY: OK, I'm gonna go for it,\nlike a big wave.\pI love you! Whew. Said it."),
        },
        .proposal = COMPOUND_STRING("BRAWLY: Yes! Radical! Let's ride\nthis wave forever!"),
        .whatCouldHaveBeen = COMPOUND_STRING("BRAWLY: Hey, congrats! No hard\nfeelings. Just gonna go surf."),
    },
    [COMPANION_SIDNEY] =
    {
        .name = COMPOUND_STRING("SIDNEY"),
        .graphicsId = OBJ_EVENT_GFX_SIDNEY,
        .trainerId = TRAINER_SIDNEY,
        .partnerId = PARTNER_SIDNEY,
        .gender = MALE,
        .unlock = COMPANION_UNLOCK_CHAMPION,
        LIKES(sLikes_Sidney),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("SIDNEY: Yo, Champ. Don't get cocky.\nI'll beat you one day."), COMPOUND_STRING("SIDNEY: This place is boring\nwithout you around.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("SIDNEY: You're alright, you know.\nDon't tell anyone I said so.") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("SIDNEY: I kinda wait for you now.\nWeird, right?") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("SIDNEY: Never thought I'd fall\nfor anyone. Then you showed up.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("SIDNEY: Hey, you're home. Good.\nMissed you.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("SIDNEY: Us against the world.\nAnd the world lost.") },
        },
        .outings =
        {
            COMPOUND_STRING("SIDNEY: Beach, huh? Not my scene.\p...But it's alright with you\nhere. Don't make it weird."),
            COMPOUND_STRING("SIDNEY: Night's the best time.\nNobody watching.\pJust us and the stars. I like it."),
            COMPOUND_STRING("SIDNEY: I'm bad at this. So I'll\njust say it straight.\pI love you. There. Your move."),
        },
        .proposal = COMPOUND_STRING("SIDNEY: ...Yeah. Yes. Obviously.\nCome here."),
        .whatCouldHaveBeen = COMPOUND_STRING("SIDNEY: Heh. Lucky them.\nWhatever. Be happy."),
    },
    [COMPANION_LANCE] =
    {
        .name = COMPOUND_STRING("LANCE"),
        .graphicsId = OBJ_EVENT_GFX_LANCE,
        .trainerId = TRAINER_VISITOR_LANCE,
        .partnerId = PARTNER_LANCE,
        .gender = MALE,
        .unlock = COMPANION_UNLOCK_VISITOR,
        .visitorId = ASCENSION_VISITOR_LANCE,
        LIKES(sLikes_Lance),
        .lines =
        {
            [COMPANION_STAGE_MET] = { COMPOUND_STRING("LANCE: I came to test Hoenn's\nChampion. I think I'll stay."), COMPOUND_STRING("LANCE: Dragonite sends its regards.") },
            [COMPANION_STAGE_FRIEND] = { COMPOUND_STRING("LANCE: Kanto seems very far away\nwhen I'm here with you.") },
            [COMPANION_STAGE_CLOSE] = { COMPOUND_STRING("LANCE: Dragons are loyal for life.\nI've always admired that.") },
            [COMPANION_STAGE_PARTNER] = { COMPOUND_STRING("LANCE: My heart is yours, as a\ndragon's is to its trainer.") },
            [COMPANION_STAGE_SPOUSE] = { COMPOUND_STRING("LANCE: Welcome home. Dragonite\nkept watch while you were out.") },
            [COMPANION_STAGE_ASCENDED] = { COMPOUND_STRING("LANCE: Legends, the two of us.\nAs it should be.") },
        },
        .outings =
        {
            COMPOUND_STRING("LANCE: Gyarados loves this sea.\pI find I love it too.\nOr maybe it's the company."),
            COMPOUND_STRING("LANCE: Dragons fly by starlight.\pIf I could fly anywhere tonight,\nI'd stay right here."),
            COMPOUND_STRING("LANCE: I've faced every Champion\nin every region.\pNone of them took my heart.\nYou did. I love you."),
        },
        .proposal = COMPOUND_STRING("LANCE: Yes. I swear it on my\ncape and my dragons."),
        .whatCouldHaveBeen = COMPOUND_STRING("LANCE: I'm glad for you both.\nA Champion deserves happiness."),
    },
};

#undef LIKES

// Rumours nurses may share about the player and their closest companion.
// {STR_VAR_1} is the companion's name.
static const u8 *const sRumours[COMPANION_STAGE_COUNT][2] =
{
    [COMPANION_STAGE_FRIEND] = {
        COMPOUND_STRING("Was that {STR_VAR_1} I saw walking\non the beach with you?"),
        COMPOUND_STRING("{STR_VAR_1} talks about you a lot,\nyou know."),
    },
    [COMPANION_STAGE_CLOSE] = {
        COMPOUND_STRING("Someone saw you and {STR_VAR_1}\nwatching the stars. Romantic!"),
        COMPOUND_STRING("{STR_VAR_1} smiles when your name\ncomes up. Everyone's noticed."),
    },
    [COMPANION_STAGE_PARTNER] = {
        COMPOUND_STRING("You and {STR_VAR_1}! The whole\nregion is talking about it."),
        COMPOUND_STRING("When's the wedding? Asking for\nthe whole town."),
    },
    [COMPANION_STAGE_SPOUSE] = {
        COMPOUND_STRING("Congratulations on the wedding!\nYou and {STR_VAR_1} look so happy."),
        COMPOUND_STRING("The Champion, married to\n{STR_VAR_1}. Like a fairy tale."),
    },
    [COMPANION_STAGE_ASCENDED] = {
        COMPOUND_STRING("People leave flowers at the League\nfor you and {STR_VAR_1}."),
        COMPOUND_STRING("They say {STR_VAR_1} glows now.\nLike something out of a legend."),
    },
};
