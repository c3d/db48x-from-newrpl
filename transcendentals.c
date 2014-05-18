#include "newrpl.h"

/* Transcendental functions with variable precision implementing a decimal variant of the CORDIC
 * method, as published on the paper:
 * "Computation of Decimal Transcendental Functions Using the CORDIC Algorithm"
 * By Alvaro Vazquez, Julio Villalba and Elisardo Antelo
 *
 * The implementation was done from scratch by the newRPL Team.
 *
 */

// TRANSCENDENTAL FUNCTIONS TABLES
extern uint32_t atan_1_dict[];
extern uint16_t atan_1_offsets[];
extern uint8_t  atan_1_stream[];

extern uint32_t atan_2_dict[];
extern uint16_t atan_2_offsets[];
extern uint8_t  atan_2_stream[];

extern uint32_t atan_5_dict[];
extern uint16_t atan_5_offsets[];
extern uint8_t  atan_5_stream[];

extern uint32_t atanh_1_dict[];
extern uint16_t atanh_1_offsets[];
extern uint8_t  atanh_1_stream[];

extern uint32_t atanh_2_dict[];
extern uint16_t atanh_2_offsets[];
extern uint8_t  atanh_2_stream[];

extern uint32_t atanh_5_dict[];
extern uint16_t atanh_5_offsets[];
extern uint8_t  atanh_5_stream[];

extern uint32_t cordic_K_dict[];
extern uint16_t cordic_K_offsets[];
extern uint8_t  cordic_K_stream[];

extern uint32_t cordic_Kh_dict[];
extern uint16_t cordic_Kh_offsets[];
extern uint8_t  cordic_Kh_stream[];


// TRANSCENDENTAL CONSTANTS

static const uint32_t const Constant_2PI[]={
815790254, 19893152, 95605518, 199162678, 173166529, 792482778, 552931514, 383457493, 191272874, 742757392, 989009424, 724407838, 261348855, 442132237, 393057004, 389120848, 825697725, 596979216, 41023478, 12845025, 272190136, 190776757, 336290820, 765593595, 217156876, 446558357, 360997725, 920330693, 534358098, 50851377, 436854510, 419843844, 713897112, 372503796, 909552483, 478828666, 704664781, 369893687, 426695148, 231470510, 709632272, 288394713, 954470028, 434643442, 229049838, 329412003, 558760016, 578195554, 683514934, 572510363, 485308505, 713273961, 366959550, 99823976, 47296133, 440551192, 233068997, 860591064, 442159501, 399138185, 917717385, 967701098, 911141349, 57236594, 917263006, 328243984, 201868344, 470956327, 984559356, 854520853, 652782839, 49499294, 7117528, 250676486, 630038702, 204811607, 432411393, 830420950, 735405797, 898258662, 336851813, 929241609, 420809506, 954945369, 889651075, 353567489, 119239789, 36388591, 532020942, 320712741, 802571672, 801969764, 540334227, 12080185, 987863851, 350927492, 815019676, 345571177, 906623372, 139190165, 448490830, 303114971, 56695826, 249944355, 519882778, 799154724, 60370593, 60390407, 365593646, 187230676, 172655773, 440213097, 787619051, 843284039, 322239181, 600385575, 245361322, 556106434, 391556371, 757518750, 572776470, 746231912, 85751093, 719650698, 533829460, 284123435, 506641676, 750577317, 627567, 237634202, 7052386, 650668937, 528504461, 69381660, 4891891, 219263719, 946346563, 609990211, 999674595, 442269999, 992103741, 954954261, 631962725, 172806883, 425804392, 403991222, 851784708, 158455937, 917074210, 686029309, 180244990, 744293688, 834727435, 551557921, 557154268, 52712165, 113625429, 881026400, 963693533, 350476934, 435258635, 554107843, 972188740, 438140435, 279044947, 204278989, 131328617, 734672488, 389825966, 587636602, 544978245, 470377150, 992549913, 614892475, 621023709, 386522358, 437223476, 918390618, 454073151, 321886611, 390388302, 304276829, 109764093, 2266106, 578518072, 343072873, 565850818, 304184192, 117634976, 401321263, 254744917, 452049828, 642678721, 90865329, 692069722, 129713384, 240381829, 356633054, 129646757, 689225695, 195133186, 392885762, 790986076, 928924589, 704221111, 820540387, 223490056, 881625696, 446345071, 921910116, 329418768, 302656461, 596429617, 68423413, 725606965, 257241799, 461563281, 194988918, 875021164, 839433879, 655900576, 692528676, 717958647, 628318530
};
static const uint32_t const Constant_PI[]={
407895127, 9946576, 47802759, 599581339, 86583264, 396241389, 776465757, 191728746, 95636437, 371378696, 494504712, 862203919, 630674427, 221066118, 196528502, 694560424, 412848862, 298489608, 520511739, 6422512, 636095068, 95388378, 668145410, 382796797, 608578438, 723279178, 680498862, 460165346, 767179049, 25425688, 218427255, 209921922, 356948556, 686251898, 454776241, 739414333, 852332390, 184946843, 213347574, 115735255, 854816136, 144197356, 477235014, 217321721, 614524919, 164706001, 279380008, 289097777, 841757467, 786255181, 742654252, 356636980, 183479775, 549911988, 23648066, 720275596, 116534498, 930295532, 721079750, 699569092, 458858692, 983850549, 455570674, 28618297, 458631503, 164121992, 600934172, 235478163, 992279678, 927260426, 326391419, 24749647, 3558764, 125338243, 815019351, 602405803, 216205696, 915210475, 367702898, 949129331, 668425906, 464620804, 710404753, 977472684, 944825537, 676783744, 559619894, 18194295, 766010471, 160356370, 401285836, 900984882, 770167113, 506040092, 493931925, 175463746, 907509838, 172785588, 953311686, 69595082, 724245415, 151557485, 528347913, 124972177, 259941389, 899577362, 530185296, 30195203, 182796823, 593615338, 586327886, 720106548, 893809525, 921642019, 661119590, 300192787, 122680661, 778053217, 195778185, 378759375, 286388235, 873115956, 42875546, 359825349, 766914730, 142061717, 753320838, 875288658, 313783, 118817101, 503526193, 825334468, 264252230, 534690830, 502445945, 609631859, 973173281, 804995105, 999837297, 721134999, 996051870, 977477130, 815981362, 86403441, 212902196, 201995611, 925892354, 79227968, 958537105, 343014654, 90122495, 872146844, 917363717, 275778960, 778577134, 526356082, 56812714, 940513200, 481846766, 675238467, 717629317, 277053921, 986094370, 719070217, 639522473, 602139494, 65664308, 367336244, 194912983, 793818301, 272489122, 735188575, 996274956, 807446237, 310511854, 193261179, 218611738, 959195309, 727036575, 160943305, 695194151, 652138414, 54882046, 1133053, 789259036, 171536436, 282925409, 152092096, 558817488, 700660631, 127372458, 726024914, 821339360, 45432664, 346034861, 564856692, 120190914, 678316527, 564823378, 344612847, 97566593, 196442881, 895493038, 964462294, 852110555, 410270193, 111745028, 940812848, 223172535, 460955058, 664709384, 651328230, 798214808, 534211706, 862803482, 628620899, 230781640, 97494459, 937510582, 419716939, 327950288, 846264338, 358979323, 314159265
};
static const uint32_t const Constant_PI_2[]={
203947563, 504973288, 523901379, 299790669, 543291632, 698120694, 388232878, 595864373, 47818218, 185689348, 747252356, 931101959, 315337213, 110533059, 98264251, 347280212, 206424431, 649244804, 260255869, 3211256, 318047534, 47694189, 834072705, 191398398, 304289219, 361639589, 340249431, 730082673, 383589524, 512712844, 109213627, 104960961, 178474278, 843125949, 727388120, 369707166, 926166195, 92473421, 606673787, 57867627, 427408068, 72098678, 738617507, 608660860, 807262459, 82353000, 639690004, 644548888, 920878733, 393127590, 371327126, 678318490, 91739887, 274955994, 11824033, 360137798, 58267249, 465147766, 360539875, 349784546, 729429346, 491925274, 727785337, 514309148, 229315751, 82060996, 800467086, 117739081, 496139839, 963630213, 663195709, 12374823, 501779382, 562669121, 907509675, 301202901, 608102848, 457605237, 683851449, 474564665, 334212953, 732310402, 355202376, 988736342, 472412768, 338391872, 779809947, 509097147, 383005235, 80178185, 200642918, 950492441, 385083556, 753020046, 246965962, 87731873, 453754919, 86392794, 476655843, 534797541, 862122707, 575778742, 764173956, 562486088, 129970694, 449788681, 765092648, 515097601, 91398411, 296807669, 293163943, 860053274, 946904762, 460821009, 830559795, 650096393, 561340330, 889026608, 597889092, 689379687, 143194117, 436557978, 521437773, 179912674, 883457365, 71030858, 376660419, 937644329, 500156891, 559408550, 251763096, 412667234, 132126115, 767345415, 751222972, 804815929, 986586640, 902497552, 999918648, 360567499, 498025935, 488738565, 907990681, 43201720, 606451098, 100997805, 462946177, 539613984, 479268552, 671507327, 45061247, 936073422, 458681858, 137889480, 389288567, 263178041, 28406357, 470256600, 740923383, 837619233, 858814658, 138526960, 993047185, 859535108, 319761236, 301069747, 32832154, 683668122, 597456491, 396909150, 636244561, 367594287, 998137478, 403723118, 655255927, 96630589, 609305869, 979597654, 863518287, 580471652, 347597075, 326069207, 527441023, 566526, 394629518, 585768218, 141462704, 76046048, 779408744, 350330315, 63686229, 363012457, 410669680, 522716332, 173017430, 282428346, 560095457, 339158263, 782411689, 672306423, 548783296, 98221440, 447746519, 982231147, 926055277, 205135096, 55872514, 970406424, 111586267, 230477529, 332354692, 325664115, 399107404, 267105853, 931401741, 314310449, 615390820, 48747229, 968755291, 209858469, 163975144, 923132169, 679489661, 157079632
};
static const uint32_t const Constant_PI_4[]={
19737817, 524866441, 619506897, 498953347, 716458161, 490603472, 941164393, 979321866, 239091092, 928446740, 736261780, 655509798, 576686069, 552665296, 491321255, 736401060, 32122156, 246224021, 301279348, 16056281, 590237670, 238470946, 170363525, 956991994, 521446095, 808197946, 701247156, 650413366, 917947623, 563564221, 546068137, 524804805, 892371390, 215629745, 636940604, 848535833, 630830976, 462367109, 33368935, 289338138, 137040340, 360493392, 693087535, 43304303, 36312298, 411765004, 198450020, 222744443, 604393668, 965637954, 856635631, 391592451, 458699438, 374779970, 59120166, 800688990, 291336246, 325738830, 802699377, 748922731, 647146731, 459626373, 638926687, 571545743, 146578757, 410304981, 2335430, 588695409, 480699195, 818151067, 315978549, 61874118, 508896910, 813345607, 537548377, 506014509, 40514241, 288026188, 419257247, 372823328, 671064767, 661552011, 776011883, 943681711, 362063844, 691959362, 899049736, 545485738, 915026177, 400890926, 3214590, 752462206, 925417784, 765100231, 234829813, 438659366, 268774595, 431963972, 383279215, 673987707, 310613537, 878893714, 820869782, 812430443, 649853472, 248943405, 825463242, 575488008, 456992057, 484038345, 465819716, 300266371, 734523814, 304105049, 152798977, 250481969, 806701653, 445133042, 989445464, 446898437, 715970588, 182789890, 607188867, 899563372, 417286825, 355154294, 883302095, 688221646, 500784459, 797042752, 258815482, 63336171, 660630577, 836727075, 756114863, 24079648, 932933204, 512487764, 999593244, 802837499, 490129676, 443692827, 539953407, 216008604, 32255490, 504989028, 314730885, 698069922, 396342762, 357536637, 225306238, 680367110, 293409294, 689447402, 946442835, 315890206, 142031786, 351283000, 704616917, 188096168, 294073294, 692634804, 965235925, 297675544, 598806184, 505348736, 164160771, 418340610, 987282458, 984545752, 181222806, 837971438, 990687391, 18615594, 276279637, 483152948, 46529345, 897988273, 317591439, 902358264, 737985377, 630346036, 637205116, 2832632, 973147590, 928841091, 707313522, 380230240, 897043720, 751651578, 318431146, 815062285, 53348401, 613581662, 865087152, 412141730, 800477286, 695791317, 912058446, 361532118, 743916483, 491107202, 238732595, 911155737, 630276389, 25675484, 279362571, 852032120, 557931339, 152387645, 661773461, 628320576, 995537021, 335529266, 657008706, 571552249, 76954101, 243736148, 843776455, 49292349, 819875721, 615660845, 397448309, 785398163
};

static const uint32_t const Constant_ln10[]={
258860792, 174297333, 529298781, 369821641, 644521204, 657989483, 961859875, 885040897, 968780300, 499694197, 654932371, 358841869, 911113681, 334309716, 322024482, 49698227, 668087497, 519850366, 509844717, 596619280, 850552828, 454836698, 736253322, 771898109, 935803700, 60244492, 291711937, 645994207, 226146520, 649846694, 12818533, 323629156, 141363779, 477045803, 367557981, 367426215, 370130575, 441484480, 321839530, 794346834, 59679445, 262188397, 565413047, 120032776, 493000032, 931781463, 833074669, 884819528, 164310551, 498939597, 572418187, 607512027, 497647968, 10589244, 106745244, 953708828, 745167580, 935487173, 126493347, 853794986, 686209240, 150311786, 276301794, 858079003, 920906943, 395491804, 495795491, 656829870, 661626292, 135478757, 776072664, 87456092, 298215304, 96360426, 252877833, 324563091, 711824675, 685591644, 431952393, 179866639, 881759792, 252189489, 104239783, 11101073, 178213766, 773114504, 177927636, 217785054, 736600916, 898217440, 698371632, 976336583, 89704281, 484654619, 159611356, 755570315, 681633572, 170265441, 988039194, 865092900, 250374729, 129223079, 843831919, 231010759, 107184669, 593466624, 813114691, 444500997, 975119385, 980782805, 91670584, 713456862, 787488737, 921988499, 108625714, 872869651, 341972652, 392051446, 395072385, 802051893, 708886921, 975103749, 395651876, 577097505, 182375253, 992918243, 190956102, 850456480, 98274823, 781170543, 438635917, 218084275, 473315414, 11978575, 237103331, 950878075, 643095059, 67263171, 653832076, 855097852, 827985025, 94293225, 726624257, 645497347, 221107182, 629769795, 676369386, 922147765, 149510504, 979650047, 44225071, 741834857, 524967166, 276686059, 757753970, 96762712, 448719278, 485249486, 842893148, 828084811, 401682692, 708474699, 720754403, 30896345, 602301057, 765871416, 192920333, 763113569, 589402567, 956890167, 82413146, 589257242, 245544057, 811364292, 734206705, 868569356, 167465505, 775026849, 706480002, 18064450, 636167921, 569476834, 734507478, 156591213, 148046637, 283552201, 677432162, 470806855, 880840126, 417480036, 210510171, 940440022, 939147961, 893431493, 436515504, 440424327, 654366747, 821988674, 622228769, 884616336, 537773262, 350530896, 319852839, 989482623, 468084379, 720832555, 168948290, 736909878, 675666628, 546508280, 863340952, 404228624, 834196778, 508959829, 23599720, 967735248, 96757260, 603332790, 862877297, 760110148, 468436420, 401799145, 299404568, 230258509
};

static const uint32_t const Constant_ln10_2[]= {
629430396, 587148666, 764649390, 184910820, 822260602, 828994741, 980929937, 442520448, 984390150, 749847098, 827466185, 679420934, 455556840, 167154858, 661012241, 524849113, 334043748, 759925183, 254922358, 298309640, 425276414, 227418349, 868126661, 385949054, 467901850, 530122246, 645855968, 322997103, 113073260, 824923347, 6409266, 661814578, 570681889, 738522901, 683778990, 683713107, 185065287, 220742240, 160919765, 897173417, 529839722, 631094198, 282706523, 60016388, 746500016, 965890731, 416537334, 942409764, 582155275, 749469798, 786209093, 303756013, 248823984, 5294622, 53372622, 476854414, 872583790, 967743586, 63246673, 426897493, 343104620, 75155893, 638150897, 929039501, 460453471, 697745902, 247897745, 328414935, 830813146, 67739378, 388036332, 43728046, 149107652, 548180213, 626438916, 662281545, 355912337, 842795822, 715976196, 89933319, 940879896, 626094744, 552119891, 5550536, 89106883, 386557252, 88963818, 108892527, 368300458, 449108720, 849185816, 988168291, 544852140, 242327309, 579805678, 377785157, 840816786, 85132720, 494019597, 932546450, 625187364, 564611539, 921915959, 615505379, 53592334, 796733312, 906557345, 722250498, 987559692, 490391402, 45835292, 856728431, 893744368, 460994249, 554312857, 436434825, 170986326, 696025723, 697536192, 901025946, 854443460, 487551874, 697825938, 788548752, 591187626, 496459121, 95478051, 925228240, 549137411, 890585271, 719317958, 109042137, 736657707, 505989287, 618551665, 975439037, 821547529, 33631585, 326916038, 927548926, 913992512, 547146612, 863312128, 322748673, 610553591, 314884897, 838184693, 461073882, 574755252, 989825023, 522112535, 370917428, 762483583, 138343029, 378876985, 48381356, 224359639, 242624743, 921446574, 414042405, 700841346, 854237349, 860377201, 515448172, 301150528, 882935708, 596460166, 881556784, 794701283, 478445083, 41206573, 794628621, 122772028, 905682146, 367103352, 934284678, 583732752, 387513424, 353240001, 509032225, 318083960, 284738417, 867253739, 578295606, 574023318, 141776100, 838716081, 235403427, 440420063, 708740018, 105255085, 970220011, 969573980, 446715746, 718257752, 720212163, 327183373, 910994337, 311114384, 442308168, 268886631, 675265448, 659926419, 994741311, 734042189, 360416277, 84474145, 368454939, 337833314, 273254140, 431670476, 202114312, 917098389, 254479914, 11799860, 483867624, 48378630, 801666395, 431438648, 380055074, 734218210, 200899572, 649702284, 115129254
};

static const uint32_t const Constant_One[]={ 1 };


// PUT THE CONSTANTS 2*PI,PI, PI/2 AND PI/4 RESPECTIVELY ON AN mpd_t THAT HAS ***NO MEMORY*** ALLOCATED TO IT
// DO NOT USE WITH RREG REGISTERS

// POINT THE DATA DIRECTLY INTO THE COMPRESSED DICTIONARY, SINCE THESE CONSTANTS
// CANNOT BE COMPRESSED

#define Constant_K1 &(cordic_K_dict[222*256+7])

#define Constant_Kh1 &(cordic_Kh_dict[221*256+49])

void const_2PI(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_2PI;
    real->digits=2016;
    real->exp=-2015;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}
void const_PI(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_PI;
    real->digits=2016;
    real->exp=-2015;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}
void const_PI_2(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_PI_2;
    real->digits=2016;
    real->exp=-2015;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}
void const_PI_4(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_PI_4;
    real->digits=2016;
    real->exp=-2016;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}

void const_ln10(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_ln10;
    real->digits=2016;
    real->exp=-2015;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}

void const_ln10_2(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_ln10_2;
    real->digits=2016;
    real->exp=-2015;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}

void const_K1(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_K1;
    real->digits=2016;
    real->exp=-2016;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}

void const_Kh1(mpd_t *real)
{
    real->alloc=224;
    real->data=Constant_Kh1;
    real->digits=2016;
    real->exp=-2015;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=224;
}

void const_One(mpd_t *real)
{
    real->alloc=1;
    real->data=Constant_One;
    real->digits=1;
    real->exp=0;
    real->flags=MPD_STATIC|MPD_CONST_DATA;
    real->len=1;
}



// EXTRACT A NUMBER FROM A COMPRESSED STREAM

static void decompress_number(uint8_t *stream, uint32_t *dictionary, uint32_t *data, uint32_t *enddata)
{
    int _index, repeat,len,len2,idx;

    // 1-byte w/LOWER 4-BITS = MATCH LENGTH-1 (1-16 WORDS)
    // UPPER 4-BITS = REPEAT COUNT-1 (1-16 TIMES)
    // 2-BYTES = OFFSET INTO DICTIONARY
    // IF REPEAT COUNT==15 --> MORE REPEAT BYTES FOLLOWS
    // IF MATCH LENGTH==15 --> MORE MATCH LENGTH FOLLOWS


    while(data<enddata) {
        len=*stream;
        ++stream;
        _index=*stream|(stream[1]<<8);
        stream+=2;

        if(len>>4==0xf) repeat=*stream++;
        else repeat=len>>4;
        if((len&0xf)==0xf) len=*stream++;
        else len&=0xf;

        while(repeat-->=0) {
            len2=len;
            idx=_index;
            while(len2-->=0) *data++=dictionary[idx++];
        }
    }
}

// RETURN THE CORDIC CONSTANT K FOR startindex, THE STARTING INDEX IN THE CORDIC LOOP, STORED IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void const_K_table(int startindex,mpd_t *real)
{

    // WARNING: 0<=startindex, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(startindex>=REAL_PRECISION_MAX/2) {
        real->exp=0;
        real->data[0]=1;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(cordic_K_stream[cordic_K_offsets[REAL_PRECISION_MAX/2-startindex]]);

    real->exp=-REAL_PRECISION_MAX;
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,cordic_K_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}

// RETURN THE CORDIC CONSTANT K FOR startindex, THE STARTING INDEX IN THE CORDIC LOOP, STORED IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void const_Kh_table(int startindex,mpd_t *real)
{

    // WARNING: 1<=startindex, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(startindex>=REAL_PRECISION_MAX/2) {
        real->exp=0;
        real->data[0]=1;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(cordic_Kh_stream[cordic_Kh_offsets[REAL_PRECISION_MAX/2-startindex]]);

    real->exp=-REAL_PRECISION_MAX+1;
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,cordic_Kh_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}



// RETURN ATAN(1*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS

static void atan_1_table(int exponent,mpd_t *real)
{

    // WARNING: 0<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>=REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=1;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(atan_1_stream[atan_1_offsets[exponent]]);

    real->exp=-REAL_PRECISION_MAX-exponent;
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,atan_1_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}


// RETURN ATAN(2*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void atan_2_table(int exponent,mpd_t *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=2;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(atan_2_stream[atan_2_offsets[exponent-1]]);

    real->exp=-REAL_PRECISION_MAX-(exponent-1);
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,atan_2_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}

static void atan_5_table(int exponent,mpd_t *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=5;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(atan_5_stream[atan_5_offsets[exponent-1]]);

    real->exp=-REAL_PRECISION_MAX-(exponent-1);
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,atan_5_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}

// CORDIC LOOP IN ROTATIONAL MODE

// TAKES INITIAL PARAMETERS IN RREG[0], RREG[1] AND RREG[2]
// RETURNS RESULTS IN RREG[5], RREG[6], RREG[7]

static void CORDIC_Rotational(int digits,int startindex)
{
int sequence[4]={5,2,2,1};
void (*functions[4])(int,mpd_t *)={atan_5_table,atan_2_table,atan_2_table,atan_1_table};
int startidx=3;
int exponent;
uint32_t status;
mpd_t *x,*y,*z,*tmp;
mpd_t *xnext,*ynext,*znext;

// USE RReg[0]=z; RReg[1]=x; RReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

// RReg[3]=S; INITIALIZED TO 1*10^0
RReg[3].len=1;
RReg[3].digits=1;
RReg[3].flags&=MPD_DATAFLAGS;

z=&RReg[0];
x=&RReg[1];
y=&RReg[2];
znext=&RReg[5];
xnext=&RReg[6];
ynext=&RReg[7];

for(exponent=startindex;exponent<startindex+digits;++exponent)
{
    do {
    RReg[3].exp=-exponent;
    RReg[3].data[0]=sequence[startidx];
    if(z->flags&MPD_NEG) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;
    mpd_qfma(xnext,&RReg[3],y,x,&Context,&status);  // x(i+1)=x(i)-S(i)*y(i)

    RReg[3].flags^=MPD_NEG;
    mpd_qfma(ynext,&RReg[3],x,y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    functions[startidx](exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=z->flags&MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // WE FINISHED ONE STEP
    // SWAP THE POINTERS TO AVOID COPYING THE NUMBERS
    tmp=znext;
    znext=z;
    z=tmp;

    tmp=xnext;
    xnext=x;
    x=tmp;

    tmp=ynext;
    ynext=y;
    y=tmp;

    ++startidx;
    startidx&=3;
    } while(startidx);
}

// THE FINAL RESULTS ARE ALWAYS IN RREG[5], RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, ABOUT 6 OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}


// CALCULATE RReg[0]=cos(angle) and RReg[1]=sin(angle) BOTH WITH 9 DIGITS MORE THAN CURRENT SYSTEM PRECISION (ABOUT 6 OF THEM ARE GOOD DIGITS, ROUNDING IS NEEDED)

void trig_sincos(mpd_t *angle)
{
    int negsin,negcos,swap;
    mpd_t pi,pi2,pi4;

    const_PI(&pi);
    const_PI_2(&pi2);
    const_PI_4(&pi4);

    negcos=negsin=swap=0;
    // ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
    Context.prec+=MPD_RDIGITS;

    // GET ANGLE MODULO PI
    mpd_divmod(&RReg[1],&RReg[0],angle,&pi,&Context);

    // HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH

    // IF THE RESULT OF TEH DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(RReg[1].data[0]&1) { negcos=negsin=1; }

    if(RReg[0].flags&MPD_NEG) { negsin^=1; RReg[0].flags&=~MPD_NEG; }

    if(mpd_cmp(&RReg[0],&pi2,&Context)==1) {
        swap=1;
        negcos^=1;
        mpd_sub(&RReg[1],&RReg[0],&pi2,&Context);
        mpd_copy(&RReg[0],&RReg[1],&Context);
    }
    if(mpd_cmp(&RReg[0],&pi4,&Context)==1) {
        swap^=1;
        mpd_sub(&RReg[1],&pi2,&RReg[0],&Context);
        mpd_copy(&RReg[0],&RReg[1],&Context);
    }


    // USE RReg[0]=z; RReg[1]=x; RReg[2]=y;

    // y=0;
    RReg[2].len=1;
    RReg[2].data[0]=0;
    RReg[2].exp=0;
    RReg[2].flags&=MPD_DATAFLAGS;
    RReg[2].digits=1;

    // x=K;
    RReg[1].len=REAL_PRECISION_MAX/MPD_RDIGITS;
    RReg[1].digits=REAL_PRECISION_MAX;
    memcpy(RReg[1].data,Constant_K1,REAL_PRECISION_MAX/MPD_RDIGITS*sizeof(uint32_t));
    RReg[1].exp=-REAL_PRECISION_MAX;
    RReg[1].flags&=MPD_DATAFLAGS;


    CORDIC_Rotational((Context.prec>REAL_PRECISION_MAX)? REAL_PRECISION_MAX:Context.prec,0);

    // RESTORE PREVIOUS PRECISION
    Context.prec-=MPD_RDIGITS;

    // HERE WE HAVE
    // USE RReg[5]=angle_error; RReg[6]=cos(z) RReg[7]=sin(z);

    // PUT THE cos(z) IN RReg[0]
    if(swap) {
        mpd_copy(&RReg[0],&RReg[7],&Context);
        mpd_copy(&RReg[1],&RReg[6],&Context);
    }
    else {
        mpd_copy(&RReg[1],&RReg[7],&Context);
        mpd_copy(&RReg[0],&RReg[6],&Context);
    }
    if(negcos) RReg[0].flags|=MPD_NEG;
    if(negsin) RReg[1].flags|=MPD_NEG;

}



// CORDIC LOOP IN VECTORING MODE

// TAKES INITIAL PARAMETERS IN RREG[0], RREG[1] AND RREG[2]
// RETURNS RESULTS IN RREG[5], RREG[6], RREG[7]

static void CORDIC_Vectoring(int digits,int startindex)
{
int sequence[4]={5,2,2,1};
void (*functions[4])(int,mpd_t *)={atan_5_table,atan_2_table,atan_2_table,atan_1_table};
int startidx=3;
int exponent;
uint32_t status;
mpd_t *x,*y,*z,*tmp;
mpd_t *xnext,*ynext,*znext;

// USE RReg[0]=0.0; RReg[1]=x; RReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

// RReg[3]=S; INITIALIZED TO 1*10^0
RReg[3].len=1;
RReg[3].digits=1;
RReg[3].flags&=MPD_DATAFLAGS;

z=&RReg[0];
x=&RReg[1];
y=&RReg[2];
znext=&RReg[5];
xnext=&RReg[6];
ynext=&RReg[7];

for(exponent=startindex;exponent<startindex+digits;++exponent)
{
    do {
    RReg[3].exp=-exponent;
    RReg[3].data[0]=sequence[startidx];
    if(!(y->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;
    mpd_qfma(xnext,&RReg[3],y,x,&Context,&status);  // x(i+1)=x(i)-S(i)*y(i)

    RReg[3].flags^=MPD_NEG;
    mpd_qfma(ynext,&RReg[3],x,y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    functions[startidx](exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=y->flags&MPD_NEG;

    mpd_qadd(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // WE FINISHED ONE STEP
    // SWAP THE POINTERS TO AVOID COPYING THE NUMBERS
    tmp=znext;
    znext=z;
    z=tmp;

    tmp=xnext;
    xnext=x;
    x=tmp;

    tmp=ynext;
    ynext=y;
    y=tmp;

    ++startidx;
    startidx&=3;
    } while(startidx);
}

// THE FINAL RESULTS ARE ALWAYS IN RREG[5], RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, ABOUT 6 OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}




void trig_atan2(mpd_t *y0,mpd_t *x0)
{
// THE ONLY REQUIREMENT IS THAT y0 <= x0
int negx=x0->flags&MPD_NEG;
int negy=y0->flags&MPD_NEG;
int swap=0;

x0->flags^=negx;
y0->flags^=negy;
// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.prec+=MPD_RDIGITS;

if(mpd_iszero(x0)) {
    mpd_t pi_2;
    const_PI_2(&pi_2);
    mpd_copy(&RReg[5],&pi_2,&Context);
}
else {
    if(mpd_cmp(y0,x0,&Context)==1) {
        // NEED TO COMPUTE ATAN(X/Y) AND THEN CONVERT
        mpd_copy(&RReg[1],y0,&Context);
        mpd_copy(&RReg[2],x0,&Context);
        swap=1;
    } else {
        mpd_copy(&RReg[1],x0,&Context);
        mpd_copy(&RReg[2],y0,&Context);
    }

 // USE CORDIC TO COMPUTE
    // z = 0
    RReg[0].len=1;
    RReg[0].data[0]=0;
    RReg[0].exp=0;
    RReg[0].flags&=MPD_DATAFLAGS;
    RReg[0].digits=1;

    CORDIC_Vectoring((Context.prec>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+9:Context.prec,0);

}


if(swap) {
    mpd_t pi_2;
    const_PI_2(&pi_2);
    // RESULT = (PI/2 - ANGLE) FOR x0 POSITIVE
    // OR (PI/2+ANGLE) FOR x0 NEGATIVE
    if(negx) RReg[5].flags|=MPD_NEG;
    mpd_sub(&RReg[0],&pi_2,&RReg[5],&Context);
}
else {
if(negx) {
    mpd_t pi;
    const_PI(&pi);
    // RESULT = PI - ANGLE
    mpd_sub(&RReg[0],&pi,&RReg[5],&Context);
}
else {
    mpd_copy(&RReg[0],&RReg[5],&Context);
}
}

if(negy) {
    RReg[0].flags|=MPD_NEG;
}

Context.prec-=MPD_RDIGITS;

// HERE RReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}




// RETURN ATANH(1*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS

static void atanh_1_table(int exponent,mpd_t *real)
{

    // WARNING: 0<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>=REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=1;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(atanh_1_stream[atanh_1_offsets[exponent-1]]);

    real->exp=-REAL_PRECISION_MAX-(exponent-1);
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,atanh_1_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}


// RETURN ATANH(2*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void atanh_2_table(int exponent,mpd_t *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=2;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(atanh_2_stream[atanh_2_offsets[exponent-1]]);

    real->exp=-REAL_PRECISION_MAX-(exponent-1);
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,atanh_2_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}

static void atanh_5_table(int exponent,mpd_t *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=5;
        real->len=1;
        real->digits=1;
        real->flags&=MPD_DATAFLAGS;
        return;
    }

    uint8_t *byte=&(atanh_5_stream[atanh_5_offsets[exponent-1]]);

    real->exp=-REAL_PRECISION_MAX-(exponent-1);
    real->len=REAL_PRECISION_MAX/9;
    real->flags&=MPD_DATAFLAGS;
    real->digits=REAL_PRECISION_MAX;

    decompress_number(byte,atanh_5_dict,real->data,real->data+REAL_PRECISION_MAX/9);

}



// CORDIC LOOP IN HYPERBOLIC ROTATIONAL MODE

// TAKES INITIAL PARAMETERS IN RREG[0], RREG[1] AND RREG[2]
// RETURNS RESULTS IN RREG[5], RREG[6], RREG[7]

void (*functions[4])(int,mpd_t *)={atanh_5_table,atanh_2_table,atanh_2_table,atanh_1_table};
static void CORDIC_Hyp_Rotational(int digits,int startexp)
{
int sequence[4]={5,2,2,1};
int startidx=0;
int exponent;
uint32_t status;
mpd_t *x,*y,*z,*tmp;
mpd_t *xnext,*ynext,*znext;

// USE RReg[0]=z; RReg[1]=x; RReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

// RReg[3]=S; INITIALIZED TO 1*10^0
RReg[3].len=1;
RReg[3].digits=1;
RReg[3].flags&=MPD_DATAFLAGS;

z=&RReg[0];
x=&RReg[1];
y=&RReg[2];
znext=&RReg[5];
xnext=&RReg[6];
ynext=&RReg[7];

digits=(digits+1)>>1;

for(exponent=startexp;exponent<startexp+digits;++exponent)
{
    do {
    RReg[3].exp=-exponent;
    RReg[3].data[0]=sequence[startidx];
    if(!(z->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;
    mpd_qfma(xnext,&RReg[3],y,x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    //RReg[3].flags^=MPD_NEG;
    mpd_qfma(ynext,&RReg[3],x,y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)
    //mpd_qadd(ynext,&RReg[4],&RReg[3],&Context,&status);

    functions[startidx](exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=z->flags&MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // WE FINISHED ONE STEP
    // SWAP THE POINTERS TO AVOID COPYING THE NUMBERS
    tmp=znext;
    znext=z;
    z=tmp;

    tmp=xnext;
    xnext=x;
    x=tmp;

    tmp=ynext;
    ynext=y;
    y=tmp;

    ++startidx;
    startidx&=3;
    } while(startidx);
}
// THE FINAL RESULTS ARE ALWAYS IN RREG[0], RREG[1] AND RREG[2]

// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
mpd_qfma(xnext,z,y,x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
mpd_qfma(ynext,z,x,y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, NONE OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}

static void CORDIC_Hyp_Rotational_unrolled(int digits,int startexp)
{
int exponent;
uint32_t status;
mpd_t *x,*y,*z;
mpd_t *xnext,*ynext,*znext;

// USE RReg[0]=z; RReg[1]=x; RReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

z=&RReg[0];
x=&RReg[1];
y=&RReg[2];
znext=&RReg[5];
xnext=&RReg[6];
ynext=&RReg[7];

digits=(digits+1)>>1;

for(exponent=startexp;exponent<startexp+digits;++exponent)
{
    // ITERATION W/5

    // RReg[3]= (5*10^-exponent)*y
    mpd_qadd(&RReg[3],y,y,&Context,&status);
    mpd_qadd(&RReg[4],&RReg[3],&RReg[3],&Context,&status);
    mpd_qadd(&RReg[3],&RReg[4],y,&Context,&status);
    RReg[3].exp-=exponent;

    if(!(z->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;
    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],x,x,&Context,&status);
    mpd_qadd(&RReg[4],&RReg[3],&RReg[3],&Context,&status);
    mpd_qadd(&RReg[3],&RReg[4],x,&Context,&status);

    RReg[3].exp-=exponent;
    if(!(z->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;

    mpd_qadd(ynext,&RReg[3],y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_5_table(exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=z->flags&MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // FIRST ITERATION WITH 2

    // RReg[3]= (2*10^-exponent)*y
    mpd_qadd(&RReg[3],ynext,ynext,&Context,&status);
    RReg[3].exp-=exponent;

    if(!(znext->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;
    mpd_qadd(x,&RReg[3],xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],xnext,xnext,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(znext->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;

    mpd_qadd(y,&RReg[3],ynext,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_2_table(exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=znext->flags&MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // SECOND ITERATION WITH 2

    mpd_qadd(&RReg[3],y,y,&Context,&status);
    RReg[3].exp-=exponent;


    if(!(z->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;
    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],x,x,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(z->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;

    mpd_qadd(ynext,&RReg[3],y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    RReg[4].flags&=~MPD_NEG;
    RReg[4].flags|=z->flags&MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // ITERATION WITH 1
    ynext->exp-=exponent;

    if(!(znext->flags&MPD_NEG)) {
        mpd_qadd(x,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        mpd_qsub(x,xnext,ynext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    ynext->exp+=exponent;
    xnext->exp-=exponent;
    if(!(znext->flags&MPD_NEG)) {
        mpd_qadd(y,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        mpd_qsub(y,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    xnext->exp+=exponent;

    atanh_1_table(exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=znext->flags&MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

}
// THE FINAL RESULTS ARE ALWAYS IN RREG[0], RREG[1] AND RREG[2]

// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
mpd_qfma(xnext,z,y,x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
mpd_qfma(ynext,z,x,y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, NONE OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}


// SPECIAL VERSION WITH HALF THE OPERATIONS FOR EXP() FUNCTION

static void CORDIC_Hyp_Rotational_exp(int digits,int startexp)
{
int exponent;
uint32_t status;
mpd_t *x,*y,*z;
mpd_t *xnext,*ynext,*znext;

// USE RReg[0]=z; RReg[1]=x;
// THE INITIAL VALUES MUST'VE BEEN SET

z=&RReg[0];
x=&RReg[1];
znext=&RReg[5];
xnext=&RReg[6];

digits=(digits+1)>>1;

for(exponent=startexp;exponent<startexp+digits;++exponent)
{
    // ITERATION W/5

    // RReg[3]= (5*10^-exponent)*y
    mpd_qadd(&RReg[3],x,x,&Context,&status);
    mpd_qadd(&RReg[4],&RReg[3],&RReg[3],&Context,&status);
    mpd_qadd(&RReg[3],&RReg[4],x,&Context,&status);

    RReg[3].exp-=exponent;
    if(!(z->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;

    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_5_table(exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=z->flags&MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // FIRST ITERATION WITH 2

    // RReg[3]= (2*10^-exponent)*y
    mpd_qadd(&RReg[3],xnext,xnext,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(znext->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;

    mpd_qadd(x,&RReg[3],xnext,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_2_table(exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=znext->flags&MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // SECOND ITERATION WITH 2

    mpd_qadd(&RReg[3],x,x,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(z->flags&MPD_NEG)) RReg[3].flags&=~MPD_NEG;
    else RReg[3].flags|=MPD_NEG;

    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    RReg[4].flags&=~MPD_NEG;
    RReg[4].flags|=z->flags&MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // ITERATION WITH 1
    mpd_copy(&RReg[3],xnext,&Context);
    RReg[3].exp-=exponent;

    if(!(znext->flags&MPD_NEG)) {
        mpd_qadd(x,&RReg[3],xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        mpd_qsub(x,xnext,&RReg[3],&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }

    atanh_1_table(exponent,&RReg[4]);     // GET Alpha(i)
    RReg[4].flags|=znext->flags&MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

}
// THE FINAL RESULTS ARE ALWAYS IN RREG[0], RREG[1] AND RREG[2]

// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
mpd_qfma(xnext,z,x,x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, NONE OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}



// CALCULATES EXP(x0), AND RETURNS IT IN RREG[0]

void hyp_exp(mpd_t *x0)
{

int isneg;
// RANGE REDUCTION TO +/- LN(10)/2

// MAKE POSITIVE
isneg=x0->flags&MPD_NEG;
x0->flags&=~MPD_NEG;

// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.prec+=MPD_RDIGITS;
// GET ANGLE MODULO LN(10)
mpd_t ln10,ln10_2,Kh;

const_ln10(&ln10);
const_ln10_2(&ln10_2);
const_Kh1(&Kh);

mpd_divmod(&RReg[1],&RReg[0],x0,&ln10,&Context);

// HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH

// THE QUOTIENT NEEDS TO BE ADDED TO THE EXPONENT, SO IT SHOULD BE +/-30000
uint32_t status=0;
BINT64 quotient=mpd_qget_i64(&RReg[1],&status);
if(status) {
    if(isneg) rplZeroToRReg(0); // IF THE NUMBER IS SO BIG, THEN EXP(-Inf)=0
    else {
    // SET THE ERROR
    mpd_addstatus_raise(&Context,MPD_Overflow);
    // AND RETURN INFINITY
    mpd_set_infinity(&RReg[0]);
    }
    return;
}

if(mpd_cmp(&RReg[0],&ln10_2,&Context)==1) {
    // IS OUTSIDE THE RANGE OF CONVERGENCE
    // SUBTRACT ONE MORE ln(10)
    mpd_sub(&RReg[1],&RReg[0],&ln10,&Context);
    // AND ADD IT TO THE EXPONENT CORRECTION
    ++quotient;

    mpd_copy(&RReg[0],&RReg[1],&Context);

}


// z=x0
// THE REDUCED ANGLE IS ALREADY IN RReg[0]
RReg[0].flags^=isneg;

// x=y=Kh1;
/*
RReg[1].len=REAL_PRECISION_MAX/MPD_RDIGITS;
RReg[1].digits=REAL_PRECISION_MAX;
memcpy(RReg[1].data,Constant_Kh1,REAL_PRECISION_MAX/MPD_RDIGITS*sizeof(uint32_t));
RReg[1].exp=-(REAL_PRECISION_MAX-1);
RReg[1].flags&=MPD_DATAFLAGS;

RReg[2].len=REAL_PRECISION_MAX/MPD_RDIGITS;
RReg[2].digits=REAL_PRECISION_MAX;
memcpy(RReg[2].data,Constant_Kh1,REAL_PRECISION_MAX/MPD_RDIGITS*sizeof(uint32_t));
RReg[2].exp=-(REAL_PRECISION_MAX-1);
RReg[2].flags&=MPD_DATAFLAGS;
*/

RReg[1].len=RReg[2].len=1;
RReg[1].digits=RReg[2].digits=1;
RReg[1].data[0]=RReg[2].data[0]=1;
RReg[1].exp=RReg[2].exp=0;
RReg[1].flags&=MPD_DATAFLAGS;
RReg[2].flags&=MPD_DATAFLAGS;


CORDIC_Hyp_Rotational_exp((Context.prec>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+9:Context.prec,1);

// HERE RReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
mpd_mul(&RReg[0],&RReg[6],&Kh,&Context);
if(isneg) RReg[0].exp-=quotient;
else RReg[0].exp+=quotient;  // THIS CAN EXCEED THE MAXIMUM EXPONENT IN NEWRPL, IT WILL JUST DELAY THE ERROR UNTIL ROUNDING OCCURS
Context.prec-=MPD_RDIGITS;

}


// CALCULATES SINH(x0) AND COSH(x0), AND RETURNS THEM IN RREG[1] AND RREG[2]

void hyp_sinhcosh(mpd_t *x0)
{

int isneg;
int startexp;
// RANGE REDUCTION TO +/- LN(10)/2

// MAKE POSITIVE
isneg=x0->flags&MPD_NEG;
x0->flags&=~MPD_NEG;

// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.prec+=MPD_RDIGITS;
// GET ANGLE MODULO LN(10)
mpd_t ln10,ln10_2,Kh1;
BINT64 quotient;



// LOAD CONSTANT 0.1
RReg[7].data[0]=1;
RReg[7].digits=1;
RReg[7].exp=-1;
RReg[7].flags&=MPD_DATAFLAGS;
RReg[7].len=1;

if(mpd_cmp(x0,&RReg[7],&Context)==-1) {
    // WE ARE DEALING WITH SMALL ANGLES
    quotient=0;

    startexp=-x0->exp-x0->digits+1;

    if(startexp<1) startexp=1;
    mpd_copy(&RReg[0],x0,&Context);
}
else {
    const_ln10(&ln10);
    const_ln10_2(&ln10_2);
    startexp=1;

mpd_divmod(&RReg[1],&RReg[0],x0,&ln10,&Context);

// HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH
uint32_t status=0;
quotient=mpd_qget_i64(&RReg[1],&status);

// THE QUOTIENT NEEDS TO BE ADDED TO THE EXPONENT, SO IT SHOULD BE +/-30000
if(status) {
    // NUMBER IS TOO BIG! THE EXPONENT WILL OVERFLOW
    // SET THE ERROR
    mpd_addstatus_raise(&Context,MPD_Overflow);
    // AND RETURN INFINITY
    mpd_set_infinity(&RReg[1]);
    mpd_set_infinity(&RReg[2]);
    if(isneg) mpd_set_negative(&RReg[2]);

    return;
}

if(mpd_cmp(&RReg[0],&ln10_2,&Context)==1) {
    // IS OUTSIDE THE RANGE OF CONVERGENCE
    // SUBTRACT ONE MORE ln(10)
    mpd_sub(&RReg[1],&RReg[0],&ln10,&Context);
    // AND ADD IT TO THE EXPONENT CORRECTION
    ++quotient;
    mpd_copy(&RReg[0],&RReg[1],&Context);
}

}


// z=x0
// THE REDUCED ANGLE IS ALREADY IN RReg[0]
//RReg[0].flags^=isneg;


// RReg[6]=0.5;
RReg[6].len=1;
RReg[6].data[0]=5;
RReg[6].digits=1;
RReg[6].flags&=MPD_DATAFLAGS;
RReg[6].exp=-1;
// RReg[7]=0.5e-2N;
RReg[7].len=1;
RReg[7].data[0]=5;
RReg[7].digits=1;
RReg[7].flags&=MPD_DATAFLAGS;
RReg[7].exp=-1-2*quotient;

// x=(0.5+0.5e-2N)*Kh_1
// ACTUALLY x=(0.5+0.5e-2N-1) TO REDUCE ROUNDING ERRORS
mpd_add(&RReg[1],&RReg[7],&RReg[6],&Context);
// y=(0.5-0.5e-2N)*Kh_1
// ACTUALLY (0.5-0.5e-2N)
mpd_sub(&RReg[2],&RReg[6],&RReg[7],&Context);

// HERE RReg[0] = REDUCED ANGLE
// RReg[1] = x0
// RReg[2] = y0

CORDIC_Hyp_Rotational_unrolled((Context.prec>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+9:Context.prec,startexp);


// HERE:
// RReg[6] = cosh(angle)
// RReg[7] = sinh(angle)

// ADJUST BY PROPER CONSTANT Kh

const_Kh_table(startexp,&RReg[3]);

mpd_mul(&RReg[1],&RReg[6],&RReg[3],&Context);
mpd_mul(&RReg[2],&RReg[7],&RReg[3],&Context);

// ADJUST RESULT BY N
RReg[1].exp+=quotient;
RReg[2].exp+=quotient;

// AND ADJUST THE SIGN FOR SINH
RReg[2].flags^=isneg;

Context.prec-=MPD_RDIGITS;

// RETURNED RESULTS IN RREG[1] AND RREG[2]


}


// HYPERBOLIC CORDIC FUNCTION IN VECTORING MODE

static void CORDIC_Hyp_Vectoring_unrolled(int digits,int startexp)
{
int exponent;
uint32_t status;
mpd_t *x,*y,*z;
mpd_t *xnext,*ynext,*znext;

// USE RReg[0]=z; RReg[1]=x; RReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

z=&RReg[0];
x=&RReg[1];
y=&RReg[2];
znext=&RReg[5];
xnext=&RReg[6];
ynext=&RReg[7];

// VECTORING MODE REQUIRES THE FIRST STEP REPEATED N TIMES
// PASSED AS A NEGATIVE EXPONENT START
// WARNING: DO NOT USE STARTEXP<1 FOR SQUARE ROOT, BECAUSE IT CHANGES THE Kh CONSTANT
exponent=1;
while(startexp<1) {
    // ITERATION W/5

    // RReg[3]= (5*10^-exponent)*y
    mpd_qadd(&RReg[3],y,y,&Context,&status);
    mpd_qadd(&RReg[4],&RReg[3],&RReg[3],&Context,&status);
    mpd_qadd(&RReg[3],&RReg[4],y,&Context,&status);
    RReg[3].exp-=exponent;

    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],x,x,&Context,&status);
    mpd_qadd(&RReg[4],&RReg[3],&RReg[3],&Context,&status);
    mpd_qadd(&RReg[3],&RReg[4],x,&Context,&status);

    RReg[3].exp-=exponent;
    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(ynext,&RReg[3],y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_5_table(exponent,&RReg[4]);     // GET Alpha(i)
    if(!(y->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // FIRST ITERATION WITH 2

    // RReg[3]= (2*10^-exponent)*y
    mpd_qadd(&RReg[3],ynext,ynext,&Context,&status);
    RReg[3].exp-=exponent;

    if(!(ynext->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;
    mpd_qadd(x,&RReg[3],xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],xnext,xnext,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(ynext->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(y,&RReg[3],ynext,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_2_table(exponent,&RReg[4]);     // GET Alpha(i)
    if(!(ynext->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // SECOND ITERATION WITH 2

    mpd_qadd(&RReg[3],y,y,&Context,&status);
    RReg[3].exp-=exponent;


    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],x,x,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(ynext,&RReg[3],y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    RReg[4].flags&=~MPD_NEG;
    if(!(y->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // ITERATION WITH 1
    ynext->exp-=exponent;

    if(ynext->flags&MPD_NEG) {
         mpd_qadd(x,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        mpd_qsub(x,xnext,ynext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    ynext->exp+=exponent;
    xnext->exp-=exponent;
    if(ynext->flags&MPD_NEG) {
        mpd_qadd(y,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        mpd_qsub(y,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    xnext->exp+=exponent;

    atanh_1_table(exponent,&RReg[4]);     // GET Alpha(i)
    if(!(ynext->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)


++startexp;
}



for(exponent=startexp;exponent<startexp+digits;++exponent)
{
    // ITERATION W/5

    // RReg[3]= (5*10^-exponent)*y
    mpd_qadd(&RReg[3],y,y,&Context,&status);
    mpd_qadd(&RReg[4],&RReg[3],&RReg[3],&Context,&status);
    mpd_qadd(&RReg[3],&RReg[4],y,&Context,&status);
    RReg[3].exp-=exponent;

    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],x,x,&Context,&status);
    mpd_qadd(&RReg[4],&RReg[3],&RReg[3],&Context,&status);
    mpd_qadd(&RReg[3],&RReg[4],x,&Context,&status);

    RReg[3].exp-=exponent;
    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(ynext,&RReg[3],y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_5_table(exponent,&RReg[4]);     // GET Alpha(i)
    if(!(y->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // FIRST ITERATION WITH 2

    // RReg[3]= (2*10^-exponent)*y
    mpd_qadd(&RReg[3],ynext,ynext,&Context,&status);
    RReg[3].exp-=exponent;

    if(!(ynext->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;
    mpd_qadd(x,&RReg[3],xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],xnext,xnext,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(ynext->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(y,&RReg[3],ynext,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_2_table(exponent,&RReg[4]);     // GET Alpha(i)
    if(!(ynext->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // SECOND ITERATION WITH 2

    mpd_qadd(&RReg[3],y,y,&Context,&status);
    RReg[3].exp-=exponent;


    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(xnext,&RReg[3],x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)

    mpd_qadd(&RReg[3],x,x,&Context,&status);
    RReg[3].exp-=exponent;
    if(!(y->flags&MPD_NEG)) RReg[3].flags^=MPD_NEG;

    mpd_qadd(ynext,&RReg[3],y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

    RReg[4].flags&=~MPD_NEG;
    if(!(y->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(znext,z,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

    // ITERATION WITH 1
    ynext->exp-=exponent;

    if(ynext->flags&MPD_NEG) {
         mpd_qadd(x,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        mpd_qsub(x,xnext,ynext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    ynext->exp+=exponent;
    xnext->exp-=exponent;
    if(ynext->flags&MPD_NEG) {
        mpd_qadd(y,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        mpd_qsub(y,ynext,xnext,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
    }
    xnext->exp+=exponent;

    atanh_1_table(exponent,&RReg[4]);     // GET Alpha(i)
    if(!(ynext->flags&MPD_NEG)) RReg[4].flags|=MPD_NEG;

    mpd_qsub(z,znext,&RReg[4],&Context,&status);  // z(i+1)=z(i)-Alpha(i)

}
// THE FINAL RESULTS ARE ALWAYS IN RREG[0], RREG[1] AND RREG[2]

// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
//mpd_qfma(xnext,z,y,x,&Context,&status);  // x(i+1)=x(i)+S(i)*y(i)
//mpd_qfma(ynext,z,x,y,&Context,&status);  // y(i+1)=y(i)+S(i)*x(i)

// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, NONE OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}



// CALCULATES ATANH(x0), AND RETURNS IT IN RREG[0]

void hyp_atanh(mpd_t *x0)
{
// THE ONLY REQUIREMENT IS THAT y0 <= x0
int negx=x0->flags&MPD_NEG;
int startexp;
mpd_t one;

x0->flags^=negx;
// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.prec+=MPD_RDIGITS;

startexp=-x0->exp-x0->digits;

if(startexp<1) {

const_One(&one);


// CRITERIA FOR REPETITION OF INITIAL STEP
// REQUIRED IN ORDER TO INCREASE THE RANGE OF CONVERGENCE
mpd_sub(&RReg[3],&one,x0,&Context);

startexp=(RReg[3].exp+RReg[3].digits)*2;

}

    mpd_copy(&RReg[2],x0,&Context);

    RReg[1].len=1;
    RReg[1].data[0]=1;
    RReg[1].exp=0;
    RReg[1].flags&=MPD_DATAFLAGS;
    RReg[1].digits=1;

 // USE CORDIC TO COMPUTE
    // z = 0
    RReg[0].len=1;
    RReg[0].data[0]=0;
    RReg[0].exp=0;
    RReg[0].flags&=MPD_DATAFLAGS;
    RReg[0].digits=1;


    CORDIC_Hyp_Vectoring_unrolled((Context.prec>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+9:Context.prec,startexp);


if(negx) RReg[0].flags|=MPD_NEG;

Context.prec-=MPD_RDIGITS;

// HERE RReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}


// CALCULATES LN(x0), AND RETURNS IT IN RREG[0]
// ARGUMENT MUST BE POSITIVE, NO ARGUMENT CHECKS HERE

void hyp_ln(mpd_t *x0)
{
    int adjustexp;
    int startexp=1;
mpd_t ln10,one;

Context.prec+=MPD_RDIGITS;

const_ln10(&ln10);
const_One(&one);

mpd_copy(&RReg[3],x0,&Context);
adjustexp=x0->exp+(x0->digits-1);
RReg[3].exp=-(RReg[3].digits-1);    // TAKE ONLY THE MANTISSA, LEFT JUSTIFIED

// y0=A-1
mpd_sub(&RReg[2],&RReg[3],&one,&Context);
// x0=A+1
mpd_add(&RReg[1],&RReg[3],&one,&Context);

// z = 0
RReg[0].len=1;
RReg[0].data[0]=0;
RReg[0].exp=0;
RReg[0].flags&=MPD_DATAFLAGS;
RReg[0].digits=1;


CORDIC_Hyp_Vectoring_unrolled((Context.prec>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+9:Context.prec,startexp);

// ADD BACK THE EXPONENT AS LN(A)=EXP*LN(10)+LN(A')
mpd_mul_i32(&RReg[4],&ln10,adjustexp,&Context);
mpd_add(&RReg[3],&RReg[0],&RReg[0],&Context);
mpd_add(&RReg[0],&RReg[3],&RReg[4],&Context);

Context.prec-=MPD_RDIGITS;

// HERE RReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}

// USE A CORDIC LOOP TO COMPUTE THE SQUARE ROOT
// SAME AS LN, BUT WE USE THE RESULT FROM x

void hyp_sqrt(mpd_t *x0)
{
    int adjustexp;
    int startexp=1;

    mpd_t Kh1,one;
    Context.prec+=MPD_RDIGITS;

    const_One(&one);
/*
    startexp=-x0->exp-x0->digits;

    if(startexp<1) {

    // CRITERIA FOR REPETITION OF INITIAL STEP
    // REQUIRED IN ORDER TO INCREASE THE RANGE OF CONVERGENCE
    mpd_sub(&RReg[3],&one,x0,&Context);

    startexp=(RReg[3].exp+RReg[3].digits)*2;

    }
*/

mpd_copy(&RReg[3],x0,&Context);
adjustexp=x0->exp+(x0->digits-1);
RReg[3].exp=-(RReg[3].digits-1);    // TAKE ONLY THE MANTISSA, LEFT JUSTIFIED

if(adjustexp&1) {
    // MAKE IT AN EVEN EXPONENT, SO IT'S EASY TO DIVIDE BY 2
    --RReg[3].exp;
    adjustexp+=1;
}

// y0=A-1
mpd_sub(&RReg[2],&RReg[3],&one,&Context);
// x0=A+1
mpd_add(&RReg[1],&RReg[3],&one,&Context);


// z = 0
RReg[0].len=1;
RReg[0].data[0]=0;
RReg[0].exp=0;
RReg[0].flags&=MPD_DATAFLAGS;
RReg[0].digits=1;


CORDIC_Hyp_Vectoring_unrolled((Context.prec>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+9:Context.prec,startexp);

const_Kh1(&Kh1);

// ADD BACK THE EXPONENT AS sqrt(A)= 2*sqrt(xin^2-yin^2) * Kh1 * 10^(exponent/2)
mpd_mul_i32(&RReg[3],&RReg[1],5,&Context);
RReg[3].exp--;
mpd_mul(&RReg[0],&RReg[3],&Kh1,&Context);
RReg[0].exp+=adjustexp/2;

Context.prec-=MPD_RDIGITS;

// HERE RReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}


void hyp_asinh(mpd_t *x)
{
    mpd_t one;
    const_One(&one);

    Context.prec+=MPD_RDIGITS;

    mpd_mul(&RReg[1],x,x,&Context);   // 1 = x^2
    mpd_add(&RReg[7],&RReg[1],&one,&Context);   // 2 = x^2+1

    Context.prec-=MPD_RDIGITS;

    hyp_sqrt(&RReg[7]); // 7 = cosh = sqrt(sinh^2+1)

    Context.prec+=MPD_RDIGITS;

    mpd_add(&RReg[2],&RReg[0],&one,&Context);   // 2 = cosh + 1

    mpd_div(&RReg[7],x,&RReg[2],&Context); // 7 = sinh / (cosh + 1)

    Context.prec-=MPD_RDIGITS;

    hyp_atanh(&RReg[7]);

    mpd_add(&RReg[1],&RReg[0],&RReg[0],&Context);

}

void hyp_acosh(mpd_t *x)
{
    mpd_t one;
    const_One(&one);

    Context.prec+=MPD_RDIGITS;

    mpd_sub(&RReg[1],x,&one,&Context);   // 1 = x-1
    mpd_add(&RReg[2],x,&one,&Context);   // 2 = x+1
    mpd_div(&RReg[7],&RReg[1],&RReg[2],&Context);

    Context.prec-=MPD_RDIGITS;

    hyp_sqrt(&RReg[7]);

    hyp_atanh(&RReg[0]);

    mpd_add(&RReg[1],&RReg[0],&RReg[0],&Context);

}
