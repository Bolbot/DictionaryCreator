#ifndef STRING_EXAMPLES_H
#define STRING_EXAMPLES_H

namespace s_ex
{
	constexpr char cpp_program[] = u8R"(//
	//	Following is a short program design the imbecile style of writing
	//	//

	#include <algorithm>
	#include <iostream>
	#include <random>
	#include <cctype>
	#include <string>
	#include <set>

	bool roll_the_dice(double probability = 0.5)
	{
		std::random_device rdevice;
		std::uniform_real_distribution<double> distr(0.,1.);
		
		return distr(rdevice) > probability;
	}
	)";


	constexpr char eng_subtitles_1[] = u8R"(1
	00:01:55,716 --> 00:01:57,593
	This is a story of myths and gods.

	2
	00:01:57,885 --> 00:02:01,180
	A story of the fabled world,
	in which reality and the supernatural are interwoven.)";


	constexpr char eng_subtitles_2[] = u8R"(0
	00:00:03,000 --> 00:00:10,500
	{\move(10,10,190,230,100,400)\fad(0,1000)\fscx25\fscy25\t(0,6000,\fscx125\fscy125)\c&H000000&\3c&H00FFFF&}★★★★★ POOP ★★★★★

	1
	00:00:12,501 --> 00:00:29,202
	{\move(400,10,190,270,100,400)\fad(0,1000)\fscx25\fscy25\t(0,6000,\fscx125\fscy125)\c&HFF0000&\3c&H00FFFF&}★★★★★ POOP ★★★★★


	1
	00:00:53,833 --> 00:00:54,665
	Who are you?

	2
	00:00:57,750 --> 00:01:01,499
	We're with the opera troupe.

	3
	00:01:02,416 --> 00:01:05,124
	Oh, I didn't recognize you!)";


	constexpr char eng_subtitles_3[] = u8R"(1
	00:00:22,231 --> 00:00:27,944
	A NIKKATSU PRODUCTION

	2
	00:01:11,948 --> 00:01:14,115
	If you're gonna kill me,
	get it over with.

	3
	00:01:14,659 --> 00:01:17,244
	Tetsu, why don't you fight?

	4
	00:01:17,787 --> 00:01:21,456
	Surely you've got
	that cute little Colt in your pocket.

	5
	00:01:23,167 --> 00:01:24,751
	What's wrong?

	6
	00:01:25,211 --> 00:01:27,462
	You're breaking
	the Kurata gang's heart.)";


	constexpr char rom_subtitles[] = u8R"(1
	00:01:15,760 --> 00:01:19,840
	Glasul pe care-l auziţi
	nu este al meu...

	2
	00:01:19,960 --> 00:01:21,840
	este vocea minţii mele.

	3
	00:01:25,520 --> 00:01:29,840
	Nu am mai vorbit
	de la şase ani.

	4
	00:01:30,280 --> 00:01:32,840
	Nimeni nu ştie de ce.
	Nici măcar eu.)";


	constexpr char fr_subtitles[] = u8R"(
	255
	00:38:12,566 --> 00:38:15,029
	Je vous répugne.

	256
	00:38:16,677 --> 00:38:20,347
	Vous me trouvez
	bien laid.

	257
	00:38:20,453 --> 00:38:22,955
	Je ne sais pas
	mentir, la Bête.

	258
	00:38:23,998 --> 00:38:27,354
	Tout est-il ici
	à votre convenance.

	259
	00:38:27,411 --> 00:38:30,614
	Je ne me trouve pas très à l'aise,
	dans ces beaux atouts,

	260
	00:38:30,624 --> 00:38:33,196
	et je n'ai pas l'habitude
	qu'on me serve.)";


	constexpr char rus_subtitles_1[] = u8R"(0
	00:00:23,680 --> 00:00:27,355
	Нимбус фильм
	представляет

	1
	00:00:29,680 --> 00:00:32,513
	Догма 3

	2
	00:00:34,400 --> 00:00:37,836
	ПОСЛЕДНЯЯ ПЕСНЬ МИФУНЕ

	3
	00:01:01,560 --> 00:01:03,312
	Спасибо, спасибо!

	4
	00:01:05,000 --> 00:01:05,637
	Спасибо!

	5
	00:01:06,920 --> 00:01:09,115
	- Мы это сделали?
	- Да!

	6
	00:01:11,080 --> 00:01:13,514
	Перед женитьбой
	надо смотреть на тещу.)";


	constexpr char rus_subtitles_2[] = u8R"(1
	00:01:48,127 --> 00:01:50,880
	Легенда об Орфее известна.

	2
	00:01:51,327 --> 00:01:56,924
	В греческой мифологии
	Орфей - фракийский певец.

	3
	00:01:57,367 --> 00:02:00,325
	Он зачаровывал даже зверей.

	4
	00:02:00,887 --> 00:02:05,358
	Исполняя свои песни,
	он забывал о жене, Эвридике,

	5
	00:02:05,727 --> 00:02:08,195
	которую забрала смерть.)";

	constexpr char de_subtitles[] = u8R"(
	1
	00:00:21,447 --> 00:00:27,044
	Über ein Viertel des Films Metropolis
	muss als verloren gelten.

	2
	00:00:29,007 --> 00:00:33,000
	Wenige Filme sind so systematisch
	verändert, verstümmelt,

	3
	00:00:33,047 --> 00:00:37,996
	verfälscht worden wie dieser. Es wurde
	weggelassen, umgestellt, umgetitelt.
	)";


	constexpr char ascii_art[] = u8R"(=====================================================
	==========================================
	_______ad88888888888888888888888a, 
	________a88888"8888888888888888888888, 
	______,8888"__"P88888888888888888888b, 
	______d88_________`""P88888888888888888, 
	_____,8888b_______________""88888888888888, 
	_____d8P'''__,aa,______________""888888888b 
	_____888bbdd888888ba,__,I_________"88888888, 
	_____8888888888888888ba8"_________,88888888b 
	____,888888888888888888b,________,8888888888 
	____(88888888888888888888,______,88888888888, 
	____d888888888888888888888,____,8___"8888888b 
	____88888888888888888888888__.;8'"""__(888888 
	____8888888888888I"8888888P_,8"_,aaa,__888888 
	____888888888888I:8888888"_,8"__`b8d'__(88888 
	____(8888888888I'888888P'_,8)__________88888 
	_____88888888I"__8888P'__,8")__________88888 
	_____8888888I'___888"___,8"_(._.)_______88888 
	_____(8888I"_____"88,__,8"_____________,8888P 
	______888I'_______"P8_,8"_____________,88888) 
	_____(88I'__________",8"__M""""""M___,888888' 
	____,8I"____________,8(____"aaaa"___,8888888 
	___,8I'____________,888a___________,8888888) 
	__,8I'____________,888888,_______,888888888 
	_,8I'____________,8888888'`-===-'888888888' 
	,8I'____________,8888888"________88888888" 
	8I'____________,8"____88_________"888888P 
	8I____________,8'_____88__________`P888" 
	8I___________,8I______88____________"8ba,. 
	(8,_________,8P'______88______________88""8bma,. 
	_8I________,8P'_______88,______________"8b___""P8ma, 
	_(8,______,8d"________`88,_______________"8b_____`"8a 
	__8I_____,8dP_________,8X8,________________"8b.____:8b 
	__(8____,8dP'__,I____,8XXX8,________________`88,____8) 
	___8,___8dP'__,I____,8XxxxX8,_____I,_________8X8,__,8 
	___8I___8P'__,I____,8XxxxxxX8,_____I,________`8X88,I8 
	___I8,__"___,I____,8XxxxxxxxX8b,____I,________8XXX88I, 
	___`8I______I'__,8XxxxxxxxxxxxXX8____I________8XXxxXX8, 
	____8I_____(8__,8XxxxxxxxxxxxxxxX8___I________8XxxxxxXX8, 
	___,8I_____I[_,8XxxxxxxxxxxxxxxxxX8__8________8XxxxxxxxX8, 
	___d8I,____I[_8XxxxxxxxxxxxxxxxxxX8b_8_______(8XxxxxxxxxX8, 
	___888I____`8,8XxxxxxxxxxxxxxxxxxxX8_8,_____,8XxxxxxxxxxxX8 
	___8888,____"88XxxxxxxxxxxxxxxxxxxX8)8I____.8XxxxxxxxxxxxX8 
	__,8888I_____88XxxxxxxxxxxxxxxxxxxX8_`8,__,8XxxxxxxxxxxxX8" 
	__d88888_____`8XXxxxxxxxxxxxxxxxxX8'__`8,,8XxxxxxxxxxxxX8" 
	__888888I_____`8XXxxxxxxxxxxxxxxX8'____"88XxxxxxxxxxxxX8" 
	__88888888bbaaaa88XXxxxxxxxxxxXX8)______)8XXxxxxxxxxXX8" 
	__8888888I,_``""""""8888888888888888aaaaa8888XxxxxXX8" 
	__(8888888I,______________________.__```"""""88888P" 
	___88888888I,___________________,8I___8,_______I8" 
	____"""88888I,________________,8I'____"I8,____;8" 
	___________`8I,_____________,8I'_______`I8,___8) 
	____________`8I,___________,8I'__________I8__:8' 
	_____________`8I,_________,8I'___________I8__:8 )";

	constexpr char french_pangram[] = u8R"(Portez ce vieux whisky au juge blond qui fume sur son île intérieure, à côté de l'alcôve ovoïde, où les bûches se consument dans l'âtre, ce qui lui permet de penser à la cænogénèse de l'être dont il est question dans la cause ambiguë entendue à Moÿ, dans un capharnaüm qui, pense-t-il, diminue çà et là la qualité de son œuvre.)";

	constexpr char nietzsche[] = u8R"(Nicht, was die Menschheit ablösen soll in der Reihenfolge der Wesen, ist das Problem, das ich hiermit stelle (— der Mensch ist ein Ende —): sondern welchen Typus Mensch man züchten soll, wollen soll, als den höherwerthigeren, lebenswürdigeren, zukunftsgewisseren.
Dieser höherwerthigere Typus ist oft genug schon dagewesen: aber als ein Glücksfall, als eine Ausnahme, niemals als gewollt. Vielmehr ist er gerade am besten gefürchtet worden, er war bisher beinahe das Furchtbare; — und aus der Furcht heraus wurde der umgekehrte Typus gewollt, gezüchtet, erreicht: das Hausthier, das Heerdenthier, das kranke Thier Mensch, — der
Christ ... )";
}

#endif
