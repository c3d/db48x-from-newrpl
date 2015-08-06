/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

/* Transcendental functions with variable precision implementing a decimal variant of the CORDIC
 * method, as published on the paper:
 * "Computation of Decimal Transcendental Functions Using the CORDIC Algorithm"
 * By Alvaro Vazquez, Julio Villalba and Elisardo Antelo
 *
 * The implementation was done from scratch by the newRPL Team.
 *
 */

// DECIMAL TRANSCENDENTAL FUNCTIONS
#include <decimal.h>
#include <stdint.h>

// TRANSCENDENTAL FUNCTIONS TABLES
extern const uint32_t atan_1_8_dict[];
extern const uint16_t atan_1_8_offsets[];
extern const uint8_t  atan_1_8_stream[];

extern const uint32_t atan_2_8_dict[];
extern const uint16_t atan_2_8_offsets[];
extern const uint8_t  atan_2_8_stream[];

extern const uint32_t atan_5_8_dict[];
extern const uint16_t atan_5_8_offsets[];
extern const uint8_t  atan_5_8_stream[];

extern const uint32_t atanh_1_8_dict[];
extern const uint16_t atanh_1_8_offsets[];
extern const uint8_t  atanh_1_8_stream[];

extern const uint32_t atanh_2_8_dict[];
extern const uint16_t atanh_2_8_offsets[];
extern const uint8_t  atanh_2_8_stream[];

extern const uint32_t atanh_5_8_dict[];
extern const uint16_t atanh_5_8_offsets[];
extern const uint8_t  atanh_5_8_stream[];

extern const uint32_t cordic_K_8_dict[];
extern const uint16_t cordic_K_8_offsets[];
extern const uint8_t  cordic_K_8_stream[];

extern const uint32_t cordic_Kh_8_dict[];
extern const uint16_t cordic_Kh_8_offsets[];
extern const uint8_t  cordic_Kh_8_stream[];






#define REAL_PRECISION_MAX MAX_PRECISION
extern REAL decRReg[10];


// TRANSCENDENTAL CONSTANTS

static const BINT const Constant_2PI[]= {
89367967, 15790253, 98931528, 60551801, 62678095, 65291991, 77817316, 14792482, 35529315, 38345749, 91272874, 27573921, 942474, 7838989, 88557244, 23726134, 4442132, 83930570, 38912084, 25697725, 69792168, 2347859, 45025041, 1360128, 75727219, 20190776, 53362908, 76559359, 17156876, 65583572, 99772544, 30693360, 80989203, 37753435, 10050851, 44368545, 41984384, 13897112, 25037967, 55248337, 28666909, 47814788, 68770466, 48369893, 4266951, 23147051, 9632272, 83947137, 47002828, 43442954, 98384346, 322904, 16329412, 45587600, 57819555, 83514934, 25103636, 30850557, 73961485, 95507132, 97636695, 33099823, 20472961, 44055119, 33068997, 5910642, 15950186, 38185442, 73853991, 9891771, 49967701, 49111413, 5723659, 17263006, 82439849, 86834432, 56327201, 93564709, 85398455, 39854520, 46527828, 4949929, 7117528, 6764860, 3870225, 11607630, 13932048, 95043241, 97830420, 27354057, 89825866, 36851813, 92416093, 80950692, 45369420, 10759549, 48988965, 89353567, 11192397, 3638859, 32020942, 7127415, 57167232, 69764802, 42278019, 18554033, 51012080, 29878638, 35092749, 15019676, 55711778, 62337234, 90165906, 8301391, 97144849, 26303114, 50566958, 24994435, 19882778, 91547245, 37059379, 90407060, 36460603, 67636559, 73187230, 71726557, 44021309, 87619051, 32840397, 23918184, 85575322, 13226003, 43424536, 71556106, 3915563, 75751875, 72776470, 62319125, 75109374, 50698085, 94607196, 43553382, 76284123, 75066416, 75057731, 627567, 76342020, 5238623, 68937007, 44616506, 66052850, 91069381, 90048918, 21926371, 46346563, 99902119, 67459560, 69999999, 37414422, 26199210, 25954954, 36319627, 17280688, 25804392, 39912224, 78470840, 55937851, 42101584, 30991707, 90686029, 81802449, 74429368, 34727435, 15579218, 15426855, 12165557, 54290527, 40011362, 33881026, 49636935, 35047693, 35258635, 41078434, 18874055, 40435972, 49474381, 98927904, 17204278, 81313286, 73467248, 89825966, 76366023, 97824558, 77150544, 99134703, 47599254, 9614892, 86210237, 38652235, 37223476, 83906184, 7315191, 86611454, 83023218, 82939038, 93304276, 61097640, 226610, 78518072, 30728735, 85081834, 84192565, 49763041, 26311763, 17401321, 82547449, 45204982, 42678721, 8653296, 6972209, 13384692, 18291297, 5424038, 57356633, 51296467, 68922569, 95133186, 28857621, 98607639, 24589790, 11119289, 38770422, 56820540, 62234900, 88162569, 46345071, 19101164, 41876892, 56461329, 96173026, 41359642, 65068423, 97256069, 25724179, 61563281, 49889184, 2116419, 33879875, 5768394, 67665590, 47692528, 7179586, 62831853
};

static const BINT const Constant_PI[]= {
94683984, 7895126, 99465764, 80275900, 81339047, 32645995, 38908658, 57396241, 67764657, 19172874, 95636437, 13786960, 50471237, 3919494, 44278622, 11863067, 2221066, 41965285, 69456042, 12848862, 84896084, 51173929, 22512520, 50680064, 37863609, 10095388, 76681454, 38279679, 8578438, 32791786, 49886272, 65346680, 90494601, 68876717, 55025425, 22184272, 20992192, 56948556, 62518983, 77624168, 14333454, 23907394, 84385233, 74184946, 52133475, 11573525, 54816136, 41973568, 23501414, 21721477, 49192173, 161452, 8164706, 72793800, 28909777, 41757467, 62551818, 65425278, 36980742, 97753566, 98818347, 66549911, 60236480, 72027559, 16534498, 2955321, 7975093, 69092721, 86926995, 54945885, 74983850, 74555706, 2861829, 58631503, 41219924, 93417216, 78163600, 96782354, 42699227, 19927260, 73263914, 2474964, 3558764, 53382430, 1935112, 5803815, 56966024, 47521620, 98915210, 13677028, 94912933, 68425906, 46208046, 40475346, 72684710, 55379774, 74494482, 94676783, 55596198, 1819429, 66010471, 3563707, 28583616, 84882401, 71139009, 9277016, 25506040, 64939319, 17546374, 7509838, 27855889, 31168617, 95082953, 54150695, 48572424, 13151557, 75283479, 12497217, 59941389, 95773622, 18529689, 95203530, 68230301, 33818279, 86593615, 85863278, 72010654, 93809525, 16420198, 11959092, 92787661, 6613001, 21712268, 85778053, 51957781, 37875937, 86388235, 31159562, 87554687, 25349042, 47303598, 71776691, 38142061, 87533208, 87528865, 313783, 88171010, 52619311, 34468503, 22308253, 83026425, 45534690, 95024459, 60963185, 73173281, 49951059, 83729780, 34999999, 18707211, 13099605, 62977477, 18159813, 8640344, 12902196, 19956112, 89235420, 27968925, 71050792, 65495853, 95343014, 40901224, 87214684, 17363717, 57789609, 57713427, 56082778, 27145263, 20005681, 66940513, 74818467, 67523846, 17629317, 70539217, 9437027, 70217986, 24737190, 49463952, 8602139, 40656643, 36733624, 94912983, 38183011, 48912279, 88575272, 49567351, 23799627, 54807446, 93105118, 19326117, 18611738, 91953092, 3657595, 43305727, 41511609, 41469519, 46652138, 30548820, 113305, 89259036, 15364367, 92540917, 92096282, 74881520, 63155881, 58700660, 41273724, 72602491, 21339360, 54326648, 3486104, 56692346, 9145648, 52712019, 78678316, 75648233, 34461284, 97566593, 64428810, 49303819, 62294895, 5559644, 19385211, 28410270, 81117450, 94081284, 23172535, 9550582, 70938446, 28230664, 48086513, 70679821, 82534211, 98628034, 62862089, 30781640, 74944592, 51058209, 16939937, 2884197, 33832795, 23846264, 53589793, 31415926
};

static const BINT const Constant_PI_2[]= {
47341992, 3947563, 49732882, 90137950, 90669523, 16322997, 69454329, 78698120, 33882328, 59586437, 47818218, 56893480, 25235618, 1959747, 72139311, 5931533, 51110533, 20982642, 34728021, 6424431, 92448042, 25586964, 11256260, 75340032, 18931804, 5047694, 88340727, 19139839, 4289219, 16395893, 24943136, 82673340, 95247300, 84438358, 27512712, 11092136, 10496096, 78474278, 31259491, 38812084, 7166727, 61953697, 42192616, 87092473, 76066737, 5786762, 27408068, 20986784, 61750707, 60860738, 24596086, 80726, 4082353, 86396900, 64454888, 20878733, 31275909, 32712639, 18490371, 98876783, 99409173, 33274955, 80118240, 36013779, 58267249, 51477660, 53987546, 84546360, 93463497, 27472942, 37491925, 87277853, 51430914, 29315751, 20609962, 46708608, 39081800, 98391177, 21349613, 9963630, 36631957, 1237482, 1779382, 26691215, 50967556, 2901907, 28483012, 23760810, 49457605, 56838514, 47456466, 34212953, 23104023, 20237673, 36342355, 27689887, 87247241, 47338391, 77798099, 50909714, 83005235, 1781853, 64291808, 92441200, 35569504, 4638508, 62753020, 32469659, 8773187, 53754919, 63927944, 65584308, 97541476, 27075347, 74286212, 56575778, 87641739, 56248608, 29970694, 97886811, 9264844, 97601765, 84115150, 66909139, 43296807, 42931639, 86005327, 46904762, 8210099, 55979546, 96393830, 3306500, 60856134, 92889026, 75978890, 68937968, 43194117, 65579781, 43777343, 12674521, 73651799, 85888345, 19071030, 93766604, 93764432, 156891, 94085505, 76309655, 67234251, 61154126, 41513212, 72767345, 97512229, 80481592, 86586640, 24975529, 91864890, 67499999, 59353605, 56549802, 81488738, 9079906, 4320172, 6451098, 9978056, 94617710, 13984462, 85525396, 32747926, 47671507, 20450612, 93607342, 58681858, 78894804, 28856713, 78041389, 63572631, 60002840, 83470256, 37409233, 83761923, 58814658, 85269608, 4718513, 35108993, 12368595, 74731976, 54301069, 20328321, 68366812, 97456491, 69091505, 24456139, 94287636, 74783675, 11899813, 27403723, 96552559, 9663058, 9305869, 95976546, 51828797, 71652863, 70755804, 20734759, 23326069, 65274410, 56652, 94629518, 57682183, 46270458, 46048141, 87440760, 31577940, 29350330, 70636862, 36301245, 10669680, 27163324, 1743052, 28346173, 54572824, 26356009, 89339158, 37824116, 67230642, 48783296, 82214405, 74651909, 31147447, 52779822, 9692605, 14205135, 40558725, 97040642, 11586267, 4775291, 35469223, 64115332, 74043256, 85339910, 41267105, 99314017, 31431044, 15390820, 87472296, 75529104, 58469968, 51442098, 16916397, 61923132, 26794896, 15707963
};

static const BINT const Constant_PI_4[]= {
36709959, 19737817, 48664410, 50689752, 53347619, 81614989, 47271645, 93490603, 69411643, 97932186, 39091092, 84467402, 26178092, 9798736, 60696555, 29657668, 55552665, 4913212, 73640106, 32122156, 62240210, 27934824, 56281301, 76700160, 94659023, 25238470, 41703635, 95699199, 21446095, 81979465, 24715680, 13366701, 76236504, 22191794, 37563564, 55460681, 52480480, 92371390, 56297458, 94060421, 35833636, 9768485, 10963083, 35462367, 80333689, 28933813, 37040340, 4933921, 8753536, 4303693, 22980433, 403631, 20411765, 31984500, 22274444, 4393668, 56379546, 63563196, 92451856, 94383915, 97045869, 66374779, 591201, 80068899, 91336246, 57388302, 69937732, 22731802, 67317489, 37364714, 87459626, 36389266, 57154574, 46578757, 3049811, 33543041, 95409002, 91955886, 6748069, 49818151, 83159785, 6187411, 8896910, 33456075, 54837781, 14509537, 42415060, 18804051, 47288026, 84192572, 37282332, 71064767, 15520116, 1188366, 81711776, 38449436, 36236206, 36691959, 88990497, 54548573, 15026177, 8909269, 21459040, 62206003, 77847524, 23192541, 13765100, 62348298, 43865936, 68774595, 19639722, 27921543, 87707383, 35376739, 71431061, 82878893, 38208697, 81243044, 49853472, 89434056, 46324224, 88008825, 20575754, 34545699, 16484038, 14658197, 30026637, 34523814, 41050497, 79897730, 81969152, 16532504, 4280670, 64445133, 79894454, 44689843, 15970588, 27898907, 18886718, 63372607, 68258995, 29441728, 95355154, 68833020, 68822164, 784459, 70427525, 81548279, 36171258, 5770633, 7566063, 63836727, 87561148, 2407964, 32933204, 24877649, 59324451, 37499999, 96768028, 82749012, 7443692, 45399534, 21600860, 32255490, 49890280, 73088550, 69922314, 27626980, 63739634, 38357536, 2253062, 68036711, 93409294, 94474022, 44283568, 90206946, 17863158, 14203, 17351283, 87046169, 18809616, 94073294, 26348042, 23592569, 75544965, 61842976, 73659880, 71505348, 1641607, 41834061, 87282458, 45457529, 22280698, 71438181, 73918379, 59499068, 37018615, 82762796, 48315294, 46529345, 79882730, 59143989, 58264317, 53779023, 3673798, 16630346, 26372051, 283263, 73147590, 88410919, 31352292, 30240707, 37203802, 57889704, 46751651, 53184311, 81506228, 53348401, 35816620, 8715261, 41730865, 72864121, 31780047, 46695791, 89120584, 36153211, 43916483, 11072027, 73259549, 55737238, 63899111, 48463027, 71025675, 2793625, 85203212, 57931339, 23876455, 77346115, 20576661, 70216283, 26699553, 6335529, 96570087, 57155224, 76954101, 37361480, 77645524, 92349843, 57210492, 84581987, 9615660, 33974483, 78539816
};

static const BINT const Constant_ln10[]= {
59125110, 58860791, 42973332, 29878117, 21641529, 12043698, 48364452, 75657989, 79618598, 88504089, 68780300, 96941979, 93237149, 41869654, 36813588, 71691111, 82334309, 73220244, 4969822, 68087497, 98503666, 84471751, 19280509, 28285966, 69885055, 22454836, 97362533, 77189810, 35803700, 2444929, 71193706, 94207291, 65206459, 69422614, 33649846, 60128185, 32362915, 41363779, 70458031, 55798147, 26215367, 5753674, 48037013, 30441484, 43218395, 79434683, 59679445, 21883970, 41304726, 32776565, 321200, 46349300, 69931781, 88330746, 88481952, 64310551, 89395971, 41818749, 12027572, 79686075, 24449764, 44010589, 81067452, 95370882, 45167580, 54871737, 49334793, 94986126, 92408537, 78668620, 94150311, 32763017, 85807900, 20906943, 54918049, 79549139, 29870495, 62926568, 75766162, 64135478, 27760726, 8745609, 98215304, 63604262, 87783309, 63091252, 46753245, 64471182, 93685591, 94319523, 17986663, 81759792, 21894898, 23978325, 1073104, 37660111, 50417821, 36773114, 41779276, 21778505, 36600916, 82174407, 37163289, 36583698, 42819763, 61908970, 56484654, 51596113, 75557031, 81633572, 2654416, 3919417, 92900988, 47298650, 7925037, 19129223, 98438319, 23101075, 7184669, 34666241, 11469159, 997813, 93854445, 80597511, 84980782, 20916705, 71345686, 87488737, 19884997, 62571492, 69651108, 26528728, 44634197, 85392051, 33950723, 80205189, 8886921, 51037497, 65187697, 97505395, 52535770, 24318237, 2992918, 1909561, 85045648, 98274823, 11705430, 63591778, 84275438, 54142180, 57547331, 31011978, 52371033, 95087807, 43095059, 72631716, 83207606, 97852653, 50258550, 22582798, 57094293, 77266242, 64549734, 21107182, 97697952, 36938662, 47765676, 5049221, 4714951, 71979650, 70442250, 74183485, 24967166, 66860595, 75397027, 62712757, 92780967, 48644871, 48485249, 18428931, 82808481, 1682692, 84746994, 75440370, 96345720, 10570308, 41660230, 33765871, 91929203, 76311356, 89402567, 68901675, 41314695, 57242082, 40575892, 29224554, 5811364, 67342067, 86856935, 67465505, 50268491, 48000277, 64450706, 79210180, 83463616, 78569476, 37345074, 15659121, 48046637, 35522011, 43216228, 6855677, 1264708, 3688084, 71417480, 22105101, 94044002, 39147961, 34314939, 51550489, 24327436, 67474404, 67465436, 69821988, 66222287, 88461633, 37773262, 5308965, 85283935, 82623319, 43799894, 55546808, 90720832, 81689482, 73690987, 75666628, 65082806, 34095254, 28624863, 67784042, 82983419, 20508959, 80235997, 96773524, 96757260, 33327900, 87729760, 10148862, 64207601, 14546843, 68401799, 92994045, 23025850
};

static const BINT const Constant_ln10_2[]= {
79562555, 29430395, 71486666, 64939058, 10820764, 6021849, 74182226, 37828994, 89809299, 44252044, 84390150, 98470989, 46618574, 20934827, 68406794, 85845555, 41167154, 36610122, 52484911, 34043748, 99251833, 92235875, 9640254, 64142983, 34942527, 61227418, 48681266, 38594905, 67901850, 1222464, 85596853, 97103645, 32603229, 34711307, 66824923, 80064092, 66181457, 70681889, 85229015, 77899073, 13107683, 52876837, 24018506, 65220742, 71609197, 89717341, 29839722, 10941985, 70652363, 16388282, 160600, 73174650, 34965890, 44165373, 94240976, 82155275, 94697985, 20909374, 56013786, 39843037, 62224882, 22005294, 40533726, 47685441, 72583790, 77435868, 24667396, 97493063, 46204268, 89334310, 97075155, 16381508, 92903950, 60453471, 77459024, 89774569, 14935247, 31463284, 37883081, 32067739, 63880363, 4372804, 49107652, 81802131, 43891654, 81545626, 23376622, 82235591, 96842795, 97159761, 8993331, 40879896, 60947449, 11989162, 50536552, 68830055, 25208910, 18386557, 70889638, 10889252, 68300458, 91087203, 18581644, 68291849, 21409881, 30954485, 78242327, 75798056, 37778515, 40816786, 51327208, 1959708, 46450494, 73649325, 53962518, 59564611, 99219159, 61550537, 53592334, 67333120, 55734579, 50498906, 96927222, 40298755, 92490391, 10458352, 85672843, 93744368, 9942498, 31285746, 34825554, 63264364, 72317098, 92696025, 66975361, 90102594, 54443460, 75518748, 82593848, 48752697, 76267885, 12159118, 51496459, 954780, 92522824, 49137411, 5852715, 31795889, 42137719, 77071090, 28773665, 65505989, 76185516, 97543903, 21547529, 36315858, 91603803, 48926326, 25129275, 61291399, 28547146, 38633121, 32274867, 10553591, 48848976, 18469331, 73882838, 52524610, 2357475, 35989825, 85221125, 37091742, 62483583, 83430297, 87698513, 81356378, 96390483, 74322435, 74242624, 59214465, 41404240, 841346, 42373497, 37720185, 48172860, 5285154, 70830115, 66882935, 45964601, 88155678, 94701283, 84450837, 20657347, 28621041, 20287946, 14612277, 52905682, 83671033, 93428467, 83732752, 75134245, 24000138, 32225353, 39605090, 41731808, 39284738, 68672537, 57829560, 74023318, 17761005, 71608114, 3427838, 632354, 1844042, 85708740, 11052550, 97022001, 69573980, 67157469, 25775244, 12163718, 33737202, 33732718, 84910994, 83111143, 44230816, 68886631, 52654482, 92641967, 41311659, 21899947, 27773404, 45360416, 90844741, 36845493, 37833314, 32541403, 67047627, 14312431, 83892021, 91491709, 60254479, 40117998, 48386762, 48378630, 16663950, 43864880, 55074431, 82103800, 57273421, 84200899, 46497022, 11512925
};


static const BINT const Constant_One[]={ 1 };

// CONSTANTS IN ROM, SHOULD NOT BE USED AS DIVISOR UNLESS THEY ARE
// LEFT JUSTIFIED
void decconst_2PI(REAL *real)
{
    int nwords=253-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;
    real->data=(BINT *)Constant_2PI+nwords;
    real->exp=-2023+(nwords<<3);
    real->flags=F_APPROX;
    real->len=253-nwords;
}
void decconst_PI(REAL *real)
{
    int nwords=253-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;
    real->data=(BINT *)Constant_PI+nwords;
    real->exp=-2023+(nwords<<3);
    real->flags=F_APPROX;
    real->len=253-nwords;

}
void decconst_PI_2(REAL *real)
{
    int nwords=253-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;

    real->data=(BINT *)Constant_PI_2+nwords;
    real->exp=-2023+(nwords<<3);
    real->flags=F_APPROX;
    real->len=253-nwords;
}
void decconst_PI_4(REAL *real)
{
    int nwords=253-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;
    real->data=(BINT *)Constant_PI_4+nwords;
    real->exp=-2024+(nwords<<3);
    real->flags=F_APPROX;
    real->len=253-nwords;
}

void decconst_ln10(REAL *real)
{
    int nwords=253-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;

    real->data=(BINT *)Constant_ln10+nwords;
    real->exp=-2023+(nwords<<3);
    real->flags=F_APPROX;
    real->len=253-nwords;
}

void decconst_ln10_2(REAL *real)
{
    int nwords=253-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;

    real->data=(BINT *)Constant_ln10_2+nwords;
    real->exp=-2023+(nwords<<3);
    real->flags=F_APPROX;
    real->len=253-nwords;
}

void decconst_One(REAL *real)
{
    real->data=(BINT *)Constant_One;
    real->exp=0;
    real->flags=0;
    real->len=1;
}


#define Constant_K1_8 &(cordic_K_8_dict[249*256+48])

#define Constant_Kh1_8 &(cordic_Kh_8_dict[248*256+68])


void decconst_K1(REAL *real)
{
    int nwords=252-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;

    real->data=((BINT *)Constant_K1_8)+nwords;
    real->exp=-2016+(nwords<<3);
    real->flags=0;
    real->len=252-nwords;
}

void decconst_Kh1(REAL *real)
{
    int nwords=252-((Context.precdigits+7)>>2);
    if(nwords<0) nwords=0;

    real->data=((BINT *)Constant_Kh1_8)+nwords;
    real->exp=-2015+(nwords<<3);
    real->flags=0;
    real->len=252-nwords;
}


// EXTRACT A NUMBER FROM A COMPRESSED STREAM

static void decompress_number(uint8_t *stream, uint32_t *dictionary, uint32_t *data, uint32_t nwords)
{
    int _index, repeat,len,len2,idx,skip;
    uint32_t enddata=data+nwords;
    // 1-byte w/LOWER 4-BITS = MATCH LENGTH-1 (1-16 WORDS)
    // UPPER 4-BITS = REPEAT COUNT-1 (1-16 TIMES)
    // 2-BYTES = OFFSET INTO DICTIONARY
    // IF REPEAT COUNT==15 --> MORE REPEAT BYTES FOLLOWS
    // IF MATCH LENGTH==15 --> MORE MATCH LENGTH FOLLOWS

    skip=REAL_PRECISION_MAX/8-nwords;

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
            if(skip>=len2+1) { skip-=len2+1; }
            else {
            len2-=skip;
            idx+=skip;
            skip=0;
            while(len2-->=0) *data++=dictionary[idx++];
            }
        }
    }
}

// RETURN THE CORDIC CONSTANT K FOR startindex, THE STARTING INDEX IN THE CORDIC LOOP, STORED IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void const_K_table(int startindex,REAL *real)
{

    // WARNING: 0<=startindex, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(startindex>=REAL_PRECISION_MAX/2) {
        real->exp=0;
        real->data[0]=1;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(cordic_K_8_stream[cordic_K_8_offsets[REAL_PRECISION_MAX/2-startindex]]);

    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3);
    real->len=words;
    real->flags=0;

    decompress_number(byte,(uint32_t *)cordic_K_8_dict,real->data,words);

}

// RETURN THE CORDIC CONSTANT K FOR startindex, THE STARTING INDEX IN THE CORDIC LOOP, STORED IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void const_Kh_table(int startindex,REAL *real)
{

    // WARNING: 1<=startindex, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(startindex>=REAL_PRECISION_MAX/2) {
        real->exp=0;
        real->data[0]=1;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(cordic_Kh_8_stream[cordic_Kh_8_offsets[REAL_PRECISION_MAX/2-startindex]]);


    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3)+1;
    real->len=words;
    real->flags=0;


    decompress_number(byte,(uint32_t *)cordic_Kh_8_dict,real->data,words);

}



// RETURN ATAN(1*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS

static void atan_1_table(int exponent,REAL *real)
{

    // WARNING: 0<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>=REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=1;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(atan_1_8_stream[atan_1_8_offsets[exponent]]);

    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3)-exponent;
    real->len=words;
    real->flags=0;

    decompress_number(byte,(uint32_t *)atan_1_8_dict,real->data,words);

}


// RETURN ATAN(2*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/9) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void atan_2_table(int exponent,REAL *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=2;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(atan_2_8_stream[atan_2_8_offsets[exponent-1]]);

    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3)-(exponent-1);
    real->len=words;
    real->flags=0;

    decompress_number(byte,(uint32_t *)atan_2_8_dict,real->data,words);

}

static void atan_5_table(int exponent,REAL *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=5;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(atan_5_8_stream[atan_5_8_offsets[exponent-1]]);

    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3)-(exponent-1);
    real->len=words;
    real->flags=0;

    decompress_number(byte,(uint32_t *)atan_5_8_dict,real->data,words);

}



// CORDIC LOOP IN ROTATIONAL MODE

// TAKES INITIAL PARAMETERS IN RREG[0], RREG[1] AND RREG[2]
// RETURNS RESULTS IN RREG[5], RREG[6], RREG[7]

typedef void (*hypfuncptr)(int,REAL *);


static void CORDIC_Rotational(int digits,int startindex)
{
const int const sequence[4]={5,2,2,1};
const hypfuncptr const functions[4]={atan_5_table,atan_2_table,atan_2_table,atan_1_table};
int startidx=(startindex)? 0:3;
int exponent;
int tmpexp,tmpflags;
REAL *x,*y,*z,*tmp;
REAL *xnext,*ynext,*znext;

// USE decRReg[0]=z; decRReg[1]=x; decRReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

// decRReg[3]=S; INITIALIZED TO 1*10^0
decRReg[3].len=1;
decRReg[3].flags=0;


if(startindex) {
    // MOVE THE REGISTERS TO PRODUCE CONSISTENT OUTPUT ON decRReg[1] AND decRReg[2]
    z=&decRReg[5];
    x=&decRReg[6];
    y=&decRReg[7];
    znext=&decRReg[0];
    xnext=&decRReg[1];
    ynext=&decRReg[2];

    copyReal(z,znext);
    copyReal(x,xnext);
    copyReal(y,ynext);
}
else {
z=&decRReg[0];
x=&decRReg[1];
y=&decRReg[2];
znext=&decRReg[5];
xnext=&decRReg[6];
ynext=&decRReg[7];
}

digits=(digits+1)>>1;

for(exponent=startindex;exponent<startindex+digits;++exponent)
{
    do {
        tmpexp=y->exp;
        tmpflags=y->flags;
        y->exp-=exponent;
    if(!z->flags&F_NEGATIVE) y->flags^=F_NEGATIVE;
    // x(i+1)=x(i)-S(i)*y(i)
    add_real_mul(xnext,x,y,sequence[startidx]);
    y->exp=tmpexp;
    y->flags=tmpflags;
    tmpexp=x->exp;
    tmpflags=x->flags;

    // y(i+1)=y(i)+S(i)*(x(i)+1)

    if(z->flags&F_NEGATIVE) x->flags^=F_NEGATIVE;
    x->exp-=exponent;
    add_real_mul(ynext,y,x,sequence[startidx]); // y(i+1)=y(i)+S(i)*x(i)

    //y(i+1)+=S(i)
    if(z->flags&F_NEGATIVE) acc_real_int(ynext,-sequence[startidx],-exponent);
    else acc_real_int(ynext,sequence[startidx],-exponent);


    if(startidx!=2) functions[startidx](exponent,&decRReg[4]);     // GET Alpha(i)

    if(z->flags&F_NEGATIVE) add_real(znext,z,&decRReg[4]); // z(i+1)=z(i)-Alpha(i)
    else sub_real(znext,z,&decRReg[4]); // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);

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

    tmp=z;

}


// THE FINAL RESULTS ARE ALWAYS IN RREG[5], RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, ABOUT 6 OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
// y(i+1)= y(i) + z(i)*(x(i)+1)
add_real(&decRReg[4],y,z);
mul_real(&decRReg[3],x,z);
normalize(&decRReg[3]);
add_real(ynext,&decRReg[3],&decRReg[4]);

// x(i+1)=x(i)-z(i)*y(i)
z->flags^=F_NEGATIVE;
mul_real(&decRReg[3],y,z);
normalize(&decRReg[3]);
add_real(xnext,&decRReg[3],x);

// THE FINAL RESULTS ARE ALWAYS IN RREG[1]=(cos(x)-1) AND RREG[2]-sin(x)
normalize(ynext);
normalize(xnext);
}


// CALCULATE decRReg[0]=cos(angle) and decRReg[1]=sin(angle) BOTH WITH 8 DIGITS MORE THAN CURRENT SYSTEM PRECISION (ABOUT 6 OF THEM ARE GOOD DIGITS, ROUNDING IS NEEDED)

void dectrig_sincos(REAL *angle)
{
    int negsin,negcos,swap,startexp;
    REAL pi,pi2,pi4;
    BINT savedprec;

    decconst_PI(&pi);
    decconst_PI_2(&pi2);
    decconst_PI_4(&pi4);

    negcos=negsin=swap=0;

    savedprec=Context.precdigits;
    Context.precdigits=(2*savedprec+8 > REAL_PRECISION_MAX)? REAL_PRECISION_MAX:(2*savedprec+8);

    copyReal(&decRReg[0],angle);
    // TODO: GET ANGLE MODULO PI
    divmodReal(&decRReg[1],&decRReg[0],angle,&pi);

    // HERE decRReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH

    // IF THE RESULT OF THE DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(decRReg[1].data[0]&1) { negcos=negsin=1; }

    if(decRReg[0].flags&F_NEGATIVE) { negsin^=1; decRReg[0].flags&=~F_NEGATIVE; }

    if(gtReal(&decRReg[0],&pi2)) {
        swap=1;
        negcos^=1;
        sub_real(&decRReg[0],&decRReg[0],&pi2);
    }
    if(gtReal(&decRReg[0],&pi4)) {
        swap^=1;
        sub_real(&decRReg[0],&pi2,&decRReg[0]);
    }

    normalize(&decRReg[0]);

    // LOAD CONSTANT 0.1
    decRReg[7].data[0]=1;
    decRReg[7].exp=-1;
    decRReg[7].flags=0;
    decRReg[7].len=1;

    if(ltReal(&decRReg[0],&decRReg[7])) {
        // WE ARE DEALING WITH SMALL ANGLES


        startexp=-decRReg[0].exp-((decRReg[0].len-1)<<3)-sig_digits(decRReg[0].data[decRReg[0].len-1])+1;

        if(startexp<=2) startexp=0; else startexp-=2;
    }
    else startexp=0;


    if(startexp>=savedprec) {
        // VERY SMALL ANGLES
        Context.precdigits=savedprec;

        if(swap) {
            // COS = 1
            decRReg[7].data[0]=1;
            decRReg[7].exp=0;
            decRReg[7].flags=0;
            decRReg[7].len=1;
            // SIN = ANGLE
            copyReal(&decRReg[6],&decRReg[0]);
        }
        else {
        // COS = 1
        decRReg[6].data[0]=1;
        decRReg[6].exp=0;
        decRReg[6].flags=0;
        decRReg[6].len=1;
        // SIN = ANGLE
        copyReal(&decRReg[7],&decRReg[0]);
        }
    }
    else {

    // USE decRReg[0]=z; decRReg[1]=x; decRReg[2]=y;

    // y=0;
    decRReg[2].len=1;
    decRReg[2].data[0]=0;
    decRReg[2].exp=0;
    decRReg[2].flags=0;

    // x=0;
    decRReg[1].len=1;
    decRReg[1].data[0]=0;
    decRReg[1].exp=0;
    decRReg[1].flags=0;

    Context.precdigits=savedprec+8;

    CORDIC_Rotational((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,startexp);

    // HERE WE HAVE
    // USE decRReg[5]=angle_error; decRReg[1]=cos(z)-1 decRReg[2]=sin(z);
    const_K_table(startexp,&decRReg[4]);

    // PUT THE cos(z) IN decRReg[6]
    if(swap) {
        mul_real(&decRReg[6],&decRReg[2],&decRReg[4]);
        mul_real(&decRReg[5],&decRReg[1],&decRReg[4]);
        normalize(&decRReg[5]);
        add_real(&decRReg[7],&decRReg[5],&decRReg[4]);
    }
    else {
        mul_real(&decRReg[7],&decRReg[2],&decRReg[4]);
        mul_real(&decRReg[5],&decRReg[1],&decRReg[4]);
        add_real(&decRReg[6],&decRReg[5],&decRReg[4]);
    }

    // RESTORE PREVIOUS PRECISION
    Context.precdigits=savedprec;


    }

    if(negcos) decRReg[6].flags|=F_NEGATIVE;
    if(negsin) decRReg[7].flags|=F_NEGATIVE;

    // NUMBERS ARE NOT FINALIZED
    // HIGHER LEVEL ROUTINE MUST FINALIZE cos OR sin AS NEEDED
    // TO AVOID WASTING TIME
}





// CORDIC LOOP IN VECTORING MODE

// TAKES INITIAL PARAMETERS IN RREG[0], RREG[1] AND RREG[2]
// RETURNS RESULTS IN RREG[5], RREG[6], RREG[7]

static void CORDIC_Vectoring(int digits,int startindex)
{
const int const sequence[4]={5,2,2,1};
const hypfuncptr const functions[4]={atan_5_table,atan_2_table,atan_2_table,atan_1_table};
int startidx=startindex? 0:3;
int exponent,tmpexp,tmpflags;
REAL *x,*y,*z,*tmp;
REAL *xnext,*ynext,*znext;

// USE decRReg[0]=0.0; decRReg[1]=x; decRReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

z=&decRReg[0];
x=&decRReg[1];
y=&decRReg[2];
znext=&decRReg[5];
xnext=&decRReg[6];
ynext=&decRReg[7];

for(exponent=startindex;exponent<startindex+digits;++exponent)
{
    do {
    tmpexp=y->exp;
    tmpflags=y->flags;
    y->exp-=exponent;
    y->flags&=~F_NEGATIVE;
    add_real_mul(xnext,x,y,sequence[startidx]); // x(i+1)=x(i)-S(i)*y(i)
    y->flags=tmpflags;
    y->exp=tmpexp;

    //tmpflags=x->flags;
    //tmpexp=x->exp;
    x->exp-=exponent;
    if(y->flags&F_NEGATIVE) x->flags&=~F_NEGATIVE;
    else x->flags|=F_NEGATIVE;
    add_real_mul(ynext,y,x,sequence[startidx]); // y(i+1)=y(i)+S(i)*x(i)
    //x->exp=tmpexp;    // NOT NEEDED, THIS VALUE OF x IS NEVER USED AGAIN
    //x->flags=tmpflags;

    functions[startidx](exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags|=y->flags&F_NEGATIVE;

    add_real(znext,z,&decRReg[4]); // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);


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




void dectrig_atan2(REAL *y0,REAL *x0)
{
// THE ONLY REQUIREMENT IS THAT y0 <= x0
int startexp,correction;
int negx=x0->flags&F_NEGATIVE;
int negy=y0->flags&F_NEGATIVE;
int swap=0;

x0->flags^=negx;
y0->flags^=negy;
// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.precdigits+=8;

if(iszeroReal(x0)) {
    REAL pi_2;
    decconst_PI_2(&pi_2);
    copyReal(&decRReg[5],&pi_2);
}
else {
    if(cmpReal(y0,x0)==1) {
        // NEED TO COMPUTE ATAN(X/Y) AND THEN CONVERT
        copyReal(&decRReg[1],y0);
        copyReal(&decRReg[2],x0);
        swap=1;
    } else {
        copyReal(&decRReg[1],x0);
        copyReal(&decRReg[2],y0);
    }

    correction=-decRReg[1].exp-((decRReg[1].len-1)<<3)-sig_digits(decRReg[1].data[decRReg[1].len-1]);
    decRReg[1].exp+=correction;
    decRReg[2].exp+=correction;
    startexp=-decRReg[2].exp-((decRReg[2].len-1)<<3)-sig_digits(decRReg[2].data[decRReg[2].len-1]);

    if(startexp<2) startexp=0;

 // USE CORDIC TO COMPUTE
    // z = 0
    decRReg[0].len=1;
    decRReg[0].data[0]=0;
    decRReg[0].exp=0;
    decRReg[0].flags=0;

    CORDIC_Vectoring((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,startexp);

}


if(swap) {
    REAL pi_2;
    decconst_PI_2(&pi_2);
    // RESULT = (PI/2 - ANGLE) FOR x0 POSITIVE
    // OR (PI/2+ANGLE) FOR x0 NEGATIVE
    if(negx) decRReg[5].flags|=F_NEGATIVE;
    sub_real(&decRReg[0],&pi_2,&decRReg[5]);
}
else {
if(negx) {
    REAL pi;
    decconst_PI(&pi);
    // RESULT = PI - ANGLE
    sub_real(&decRReg[0],&pi,&decRReg[5]);
}
else {
    copyReal(&decRReg[0],&decRReg[5]);
}
}

if(negy) {
    decRReg[0].flags|=F_NEGATIVE;
}

Context.precdigits-=8;

// HERE decRReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}

// COMPUTE ASIN(Y) = ATAN2(Y,SQRT(1-Y^2))

void dectrig_asin(REAL *x)
{
    REAL one;
    decconst_One(&one);

    Context.precdigits+=8;

    mulReal(&decRReg[1],x,x);   // 1 = x^2
    subReal(&decRReg[7],&one,&decRReg[1]);   // 2 = 1-x^2

    Context.precdigits-=8;

    //hyp_sqrt(&decRReg[7]); // 7 = cos = sqrt(1-sin^2)

    dectrig_atan2(x,&decRReg[0]);
}

// COMPUTE ACOS(X) = ATAN2(SQRT(1-X^2),X)

void dectrig_acos(REAL *x)
{
    REAL one;
    decconst_One(&one);

    Context.precdigits+=8;

    mulReal(&decRReg[1],x,x);   // 1 = x^2
    subReal(&decRReg[7],&one,&decRReg[1]);   // 2 = 1-x^2

    Context.precdigits-=8;

    //hyp_sqrt(&decRReg[7]); // 7 = cos = sqrt(1-sin^2)

    dectrig_atan2(&decRReg[0],x);
}








// *******************************************  HYPERBOLICS ***************************************************






// RETURN ATANH(1*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/8) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS

static void atanh_1_table(int exponent,REAL *real)
{

    // WARNING: 0<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>=REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=1;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(atanh_1_8_stream[atanh_1_8_offsets[exponent-1]]);

    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3)-(exponent-1);
    real->len=words;
    real->flags=0;

    decompress_number(byte,(uint32_t *)atanh_1_8_dict,real->data,words);

}


// RETURN ATANH(2*10^-exponent) IN real.
// REAL MUST HAVE MINIMUM (REAL_PRECISION_MAX/8) WORDS OF data PREALLOCATED

// THIS FUNCTION RELIES ON TABLES BEING GENERATED FOR THE SAME REAL_PRECISION_MAX NUMBER OF DIGITS


static void atanh_2_table(int exponent,REAL *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=2;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(atanh_2_8_stream[atanh_2_8_offsets[exponent-1]]);


    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3)-(exponent-1);
    real->len=words;
    real->flags=0;

    decompress_number(byte,(uint32_t *)atanh_2_8_dict,real->data,words);

}

static void atanh_5_table(int exponent,REAL *real)
{

    // WARNING: 1<=exponent<= digits, THIS FUNCTION DOES NOT CHECK FOR ARGUMENT RANGE

    if(exponent>REAL_PRECISION_MAX/2) {
        real->exp=-exponent;
        real->data[0]=5;
        real->len=1;
        real->flags=0;
        return;
    }

    uint8_t *byte=(uint8_t *)&(atanh_5_8_stream[atanh_5_8_offsets[exponent-1]]);

    int words=(Context.precdigits>>3)+2;
    if(words>REAL_PRECISION_MAX/8) words=REAL_PRECISION_MAX/8;

    real->exp=-(words<<3)-(exponent-1);
    real->len=words;
    real->flags=0;

    decompress_number(byte,(uint32_t *)atanh_5_8_dict,real->data,words);

}



// CORDIC LOOP IN HYPERBOLIC ROTATIONAL MODE

// TAKES INITIAL PARAMETERS IN RREG[0], RREG[1] AND RREG[2]
// RETURNS RESULTS IN RREG[5], RREG[6], RREG[7]

//void (*functions[4])(int,REAL *)={atanh_5_table,atanh_2_table,atanh_2_table,atanh_1_table};

static void CORDIC_Hyp_Rotational_unrolled(int digits,int startexp)
{
int exponent;
int tmpexp,tmpflags;
REAL *x,*y,*z;
REAL *xnext,*ynext,*znext;

// USE decRReg[0]=z; decRReg[1]=x; decRReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

z=&decRReg[0];
x=&decRReg[1];
y=&decRReg[2];
znext=&decRReg[5];
xnext=&decRReg[6];
ynext=&decRReg[7];

digits=(digits+1)>>1;

for(exponent=startexp;exponent<startexp+digits;++exponent)
{
    // ITERATION W/5

    tmpexp=y->exp;
    tmpflags=y->flags;
    y->exp-=exponent;
    if(!(z->flags&F_NEGATIVE)) y->flags&=~F_NEGATIVE;
    else y->flags|=F_NEGATIVE;
    add_real_mul(xnext,x,y,5); // x(i+1)=x(i)+S(i)*y(i)

    y->exp=tmpexp;
    y->flags=tmpflags;


    x->exp-=exponent;
    if(!(z->flags&F_NEGATIVE)) x->flags&=~F_NEGATIVE;
    else x->flags|=F_NEGATIVE;

    add_real_mul(ynext,y,x,5);    // y(i+1)=y(i)+S(i)*x(i)

    atanh_5_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags|=z->flags&F_NEGATIVE;

    sub_real(znext,z,&decRReg[4]); // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);


    // FIRST ITERATION WITH 2

    tmpexp=ynext->exp;
    tmpflags=ynext->flags;
    ynext->exp-=exponent;
    if(!(znext->flags&F_NEGATIVE)) ynext->flags&=~F_NEGATIVE;
    else ynext->flags|=F_NEGATIVE;
    add_real_mul(x,xnext,ynext,2); // x(i+1)=x(i)+S(i)*y(i)

    ynext->exp=tmpexp;
    ynext->flags=tmpflags;


    xnext->exp-=exponent;
    if(!(znext->flags&F_NEGATIVE)) xnext->flags&=~F_NEGATIVE;
    else xnext->flags|=F_NEGATIVE;

    add_real_mul(y,ynext,xnext,2);    // y(i+1)=y(i)+S(i)*x(i)

    atanh_2_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=znext->flags&F_NEGATIVE;

    sub_real(z,znext,&decRReg[4]); // z(i+1)=z(i)-Alpha(i)

    normalize(x);
    normalize(y);
    normalize(z);


    // SECOND ITERATION WITH 2

    tmpexp=y->exp;
    tmpflags=y->flags;
    y->exp-=exponent;
    if(!(z->flags&F_NEGATIVE)) y->flags&=~F_NEGATIVE;
    else y->flags|=F_NEGATIVE;
    add_real_mul(xnext,x,y,2); // x(i+1)=x(i)+S(i)*y(i)

    y->exp=tmpexp;
    y->flags=tmpflags;


    x->exp-=exponent;
    if(!(z->flags&F_NEGATIVE)) x->flags&=~F_NEGATIVE;
    else x->flags|=F_NEGATIVE;

    add_real_mul(ynext,y,x,2);    // y(i+1)=y(i)+S(i)*x(i)

    //atanh_2_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=z->flags&F_NEGATIVE;

    sub_real(znext,z,&decRReg[4]); // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);


    // ITERATION WITH 1
    tmpexp=ynext->exp;
    ynext->exp-=exponent;

    if(!(znext->flags&F_NEGATIVE)) {
        add_real(x,xnext,ynext);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        sub_real(x,xnext,ynext);  // x(i+1)=x(i)+S(i)*y(i)
    }
    ynext->exp=tmpexp;
    xnext->exp-=exponent;
    if(!(znext->flags&F_NEGATIVE)) {
        add_real(y,ynext,xnext);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        sub_real(y,ynext,xnext);  // x(i+1)=x(i)+S(i)*y(i)
    }
    //xnext->exp+=exponent;

    atanh_1_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=znext->flags&F_NEGATIVE;

    sub_real(z,znext,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(x);
    normalize(y);
    normalize(z);


}
// THE FINAL RESULTS ARE ALWAYS IN RREG[0], RREG[1] AND RREG[2]

// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
mul_real(&decRReg[3],z,y);
normalize(&decRReg[3]);
add_real(xnext,x,&decRReg[3]); // x(i+1)=x(i)+S(i)*y(i)
mul_real(&decRReg[3],z,x);
normalize(&decRReg[3]);

add_real(ynext,y,&decRReg[3]);  // y(i+1)=y(i)+S(i)*x(i)
normalize(ynext);
normalize(xnext);
// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, NONE OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}


// SPECIAL VERSION WITH HALF THE OPERATIONS FOR EXP() FUNCTION

static void CORDIC_Hyp_Rotational_exp(int digits,int startexp)
{
int exponent;
REAL *x,*z;
REAL xp;
REAL *xnext,*znext;

// USE decRReg[0]=z; decRReg[1]=x;
// THE INITIAL VALUES MUST'VE BEEN SET

z=&decRReg[0];
x=&decRReg[1];
znext=&decRReg[5];
xnext=&decRReg[6];

digits=(digits+1)>>1;

for(exponent=startexp;exponent<startexp+digits;++exponent)
{
    // ITERATION W/5

    xp.data=x->data;
    xp.exp=x->exp-exponent;
    xp.flags=x->flags;
    xp.len=x->len;

    if(!(z->flags&F_NEGATIVE)) add_real_mul(xnext,x,&xp,5);
    else sub_real_mul(xnext,x,&xp,5);

    atanh_5_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=z->flags&F_NEGATIVE;

    sub_real(znext,z,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(znext);
    // FIRST ITERATION WITH 2

    xp.data=xnext->data;
    xp.exp=xnext->exp-exponent;
    xp.flags=xnext->flags;
    xp.len=xnext->len;

    if(!(znext->flags&F_NEGATIVE)) add_real_mul(x,xnext,&xp,2); // x(i+1)=x(i)+S(i)*y(i)
    else sub_real_mul(x,xnext,&xp,2);


    atanh_2_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=znext->flags&F_NEGATIVE;

    sub_real(z,znext,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)


    normalize(x);
    normalize(z);

    // SECOND ITERATION WITH 2
    xp.data=x->data;
    xp.exp=x->exp-exponent;
    xp.flags=x->flags;
    xp.len=x->len;

    if(!(z->flags&F_NEGATIVE)) add_real_mul(xnext,x,&xp,2); // y(i+1)=y(i)+S(i)*x(i)
    else sub_real_mul(xnext,x,&xp,2);

    decRReg[4].flags=z->flags&F_NEGATIVE;

    sub_real(znext,z,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(znext);


    // ITERATION WITH 1
    xp.data=xnext->data;
    xp.exp=xnext->exp-exponent;
    xp.flags=xnext->flags;
    xp.len=xnext->len;

    if(!(znext->flags&F_NEGATIVE)) {
        add_real(x,xnext,&xp);  // x(i+1)=x(i)+S(i)*y(i)
    }
    else {
        sub_real(x,xnext,&xp);  // x(i+1)=x(i)+S(i)*y(i)
    }

    atanh_1_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=znext->flags&F_NEGATIVE;

    sub_real(z,znext,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(x);
    normalize(z);

}
// THE FINAL RESULTS ARE ALWAYS IN RREG[0], RREG[1] AND RREG[2]

// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
mul_real(&decRReg[3],z,x);
normalize(&decRReg[3]);
add_real(xnext,x,&decRReg[3]);  // x(i+1)=x(i)+S(i)*y(i)

normalize(&xnext);
// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, NONE OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}



// CALCULATES EXP(x0), AND RETURNS IT IN RREG[0]

void dechyp_exp(REAL *x0)
{

int isneg;
// RANGE REDUCTION TO +/- LN(10)/2

// MAKE POSITIVE
isneg=x0->flags&F_NEGATIVE;
x0->flags&=~F_NEGATIVE;

// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.precdigits+=8;
// GET ANGLE MODULO LN(10)
REAL ln10,ln10_2,Kh;

decconst_ln10(&ln10);
decconst_ln10_2(&ln10_2);
decconst_Kh1(&Kh);

divmodReal(&decRReg[1],&decRReg[0],x0,&ln10);

// HERE decRReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH

// THE QUOTIENT NEEDS TO BE ADDED TO THE EXPONENT, SO IT SHOULD BE +/-30000
// MAKE SURE THE INTEGER IS ALIGNED AND RIGHT-JUSTIFIED
if(!inBINTRange(&decRReg[1])) {
    // TODO: RAISE OVERFLOW ERROR!
    decRReg[0].len=1;
    decRReg[0].data[0]=0;
    decRReg[0].exp=0;
    if(isneg) decRReg[0].flags=0;       // exp(-INF) = 0
        else decRReg[0].flags=F_INFINITY;   // exp(INF) = INF
    return;
}
BINT quotient=getBINTReal(&decRReg[1]);
if( (quotient>30000) || (quotient<-30000)) {
    // TODO: RAISE OVERFLOW ERROR!
    decRReg[0].len=1;
    decRReg[0].data[0]=0;
    decRReg[0].exp=0;
    if(isneg) decRReg[0].flags=0;       // exp(-INF) = 0
        else decRReg[0].flags=F_INFINITY;   // exp(INF) = INF
    return;
}

if(gtReal(&decRReg[0],&ln10_2)) {
    // IS OUTSIDE THE RANGE OF CONVERGENCE
    // SUBTRACT ONE MORE ln(10)
    sub_real(&decRReg[0],&decRReg[0],&ln10);
    normalize(&decRReg[0]);
    // AND ADD IT TO THE EXPONENT CORRECTION
    ++quotient;
}


// z=x0
// THE REDUCED ANGLE IS ALREADY IN decRReg[0]
decRReg[0].flags^=isneg;

// x=y=Kh1;
/*
decRReg[1].len=REAL_PRECISION_MAX/MPD_RDIGITS;
decRReg[1].digits=REAL_PRECISION_MAX;
memcpy(decRReg[1].data,Constant_Kh1,REAL_PRECISION_MAX/MPD_RDIGITS*sizeof(uint32_t));
decRReg[1].exp=-(REAL_PRECISION_MAX-1);
decRReg[1].flags=0;

decRReg[2].len=REAL_PRECISION_MAX/MPD_RDIGITS;
decRReg[2].digits=REAL_PRECISION_MAX;
memcpy(decRReg[2].data,Constant_Kh1,REAL_PRECISION_MAX/MPD_RDIGITS*sizeof(uint32_t));
decRReg[2].exp=-(REAL_PRECISION_MAX-1);
decRReg[2].flags=0;
*/

decRReg[1].len=decRReg[2].len=1;
decRReg[1].data[0]=decRReg[2].data[0]=1;
decRReg[1].exp=decRReg[2].exp=0;
decRReg[1].flags=decRReg[2].flags=0;


CORDIC_Hyp_Rotational_exp((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,1);

// HERE decRReg[0] CONTAINS THE ANGLE WITH 8 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
mul_real(&decRReg[0],&decRReg[6],&Kh);
if(isneg) decRReg[0].exp-=quotient;
else decRReg[0].exp+=quotient;  // THIS CAN EXCEED THE MAXIMUM EXPONENT IN NEWRPL, IT WILL JUST DELAY THE ERROR UNTIL ROUNDING OCCURS
Context.precdigits-=8;

}


// CALCULATES SINH(x0) AND COSH(x0), AND RETURNS THEM IN RREG[1] AND RREG[2]

void dechyp_sinhcosh(REAL *x0)
{

int isneg;
int startexp;
// RANGE REDUCTION TO +/- LN(10)/2

// MAKE POSITIVE
isneg=x0->flags&F_NEGATIVE;
x0->flags&=~F_NEGATIVE;

// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.precdigits+=8;
// GET ANGLE MODULO LN(10)
REAL ln10,ln10_2;
BINT quotient;



// LOAD CONSTANT 0.1
decRReg[7].data[0]=1;
decRReg[7].exp=-1;
decRReg[7].flags=0;
decRReg[7].len=1;

if(ltReal(x0,&decRReg[7])) {
    // WE ARE DEALING WITH SMALL ANGLES
    quotient=0;

    startexp=-x0->exp-((x0->len-1)<<3)-sig_digits(x0->data[x0->len-1])+1;

    if(startexp<1) startexp=1;
    copyReal(&decRReg[0],x0);
}
else {
    decconst_ln10(&ln10);
    decconst_ln10_2(&ln10_2);
    startexp=1;

divmodReal(&decRReg[1],&decRReg[0],x0,&ln10);

// HERE decRReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH
if(!inBINTRange(&decRReg[1])) {
    // TODO: RAISE OVERFLOW ERROR!
    decRReg[1].len=decRReg[2].len=1;
    decRReg[1].data[0]=decRReg[2].data[0]=0;
    decRReg[1].exp=decRReg[2].exp=0;
    decRReg[1].flags=decRReg[2].flags=F_INFINITY;
    if(isneg) decRReg[2].flags|=F_NEGATIVE;
    return;
}
quotient=getBINTReal(&decRReg[1]);

// THE QUOTIENT NEEDS TO BE ADDED TO THE EXPONENT, SO IT SHOULD BE +/-30000
if((quotient>30000) || (quotient<-30000)) {
    // TODO: RAISE OVERFLOW ERROR!
    decRReg[1].len=decRReg[2].len=1;
    decRReg[1].data[0]=decRReg[2].data[0]=0;
    decRReg[1].exp=decRReg[2].exp=0;
    decRReg[1].flags=decRReg[2].flags=F_INFINITY;
    if(isneg) decRReg[2].flags|=F_NEGATIVE;

    return;
}

if(gtReal(&decRReg[0],&ln10_2)) {
    // IS OUTSIDE THE RANGE OF CONVERGENCE
    // SUBTRACT ONE MORE ln(10)
    subReal(&decRReg[0],&decRReg[0],&ln10);
    // AND ADD IT TO THE EXPONENT CORRECTION
    ++quotient;
}

}


// z=x0
// THE REDUCED ANGLE IS ALREADY IN decRReg[0]
//decRReg[0].flags^=isneg;


// decRReg[6]=0.5;
decRReg[6].len=1;
decRReg[6].data[0]=5;
decRReg[6].flags=0;
decRReg[6].exp=-1;
// decRReg[7]=0.5e-2N;
decRReg[7].len=1;
decRReg[7].data[0]=5;
decRReg[7].flags=0;
decRReg[7].exp=-1-2*quotient;

// x=(0.5+0.5e-2N)*Kh_1
// ACTUALLY x=(0.5+0.5e-2N-1) TO REDUCE ROUNDING ERRORS
add_real(&decRReg[1],&decRReg[7],&decRReg[6]);
normalize(&decRReg[1]);
// y=(0.5-0.5e-2N)*Kh_1
// ACTUALLY (0.5-0.5e-2N)
sub_real(&decRReg[2],&decRReg[6],&decRReg[7]);
normalize(&decRReg[2]);
// HERE decRReg[0] = REDUCED ANGLE
// decRReg[1] = x0
// decRReg[2] = y0

CORDIC_Hyp_Rotational_unrolled((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,startexp);


// HERE:
// decRReg[6] = cosh(angle)
// decRReg[7] = sinh(angle)

// ADJUST BY PROPER CONSTANT Kh

const_Kh_table(startexp,&decRReg[3]);

mul_real(&decRReg[1],&decRReg[6],&decRReg[3]);
mul_real(&decRReg[2],&decRReg[7],&decRReg[3]);

// ADJUST RESULT BY N
decRReg[1].exp+=quotient;
decRReg[2].exp+=quotient;

// AND ADJUST THE SIGN FOR SINH
decRReg[2].flags^=isneg;

Context.precdigits-=8;

// RETURNED RESULTS IN RREG[1] AND RREG[2]
// THEY ARE NOT NORMALIZED/FINALIZED!

}


// HYPERBOLIC CORDIC FUNCTION IN VECTORING MODE

static void CORDIC_Hyp_Vectoring_unrolled(int digits,int startexp)
{
int exponent;
uint32_t status;
BINT tmpexp,tmpflags;
REAL *x,*y,*z;
REAL *xnext,*ynext,*znext;

// USE decRReg[0]=z; decRReg[1]=x; decRReg[2]=y;
// THE INITIAL VALUES MUST'VE BEEN SET

z=&decRReg[0];
x=&decRReg[1];
y=&decRReg[2];
znext=&decRReg[5];
xnext=&decRReg[6];
ynext=&decRReg[7];

// VECTORING MODE REQUIRES THE FIRST STEP REPEATED N TIMES
// PASSED AS A NEGATIVE EXPONENT START
// WARNING: DO NOT USE STARTEXP<1 FOR SQUARE ROOT, BECAUSE IT CHANGES THE Kh CONSTANT
exponent=1;
while(startexp<1) {
    // ITERATION W/5

    // decRReg[3]= (5*10^-exponent)*y

    tmpflags=y->flags;
    tmpexp=y->exp;
    y->exp-=exponent;

    //if(!(y->flags&F_NEGATIVE)) decRReg[3].flags^=F_NEGATIVE;
    y->flags=F_NEGATIVE;
    add_real_mul(xnext,x,y,5); // x(i+1)=x(i)+S(i)*y(i)

    y->exp=tmpexp;
    y->flags=tmpflags;

    x->exp-=exponent;

    if(!(y->flags&F_NEGATIVE)) x->flags^=F_NEGATIVE;

    add_real_mul(ynext,y,x,5);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_5_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=y->flags&F_NEGATIVE;

    add_real(znext,z,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);

    // FIRST ITERATION WITH 2

    // decRReg[3]= (2*10^-exponent)*y

    tmpflags=ynext->flags;
    tmpexp=ynext->exp;
    ynext->exp-=exponent;

    //if(!(y->flags&F_NEGATIVE)) decRReg[3].flags^=F_NEGATIVE;
    ynext->flags=F_NEGATIVE;
    add_real_mul(x,xnext,ynext,2); // x(i+1)=x(i)+S(i)*y(i)

    ynext->exp=tmpexp;
    ynext->flags=tmpflags;

    xnext->exp-=exponent;

    if(!(ynext->flags&F_NEGATIVE)) xnext->flags^=F_NEGATIVE;

    add_real_mul(y,ynext,xnext,2);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_2_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=ynext->flags&F_NEGATIVE;

    add_real(z,znext,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(x);
    normalize(y);
    normalize(z);


    // SECOND ITERATION WITH 2


    tmpflags=y->flags;
    tmpexp=y->exp;
    y->exp-=exponent;

    //if(!(y->flags&F_NEGATIVE)) decRReg[3].flags^=F_NEGATIVE;
    y->flags=F_NEGATIVE;
    add_real_mul(xnext,x,y,2); // x(i+1)=x(i)+S(i)*y(i)

    y->exp=tmpexp;
    y->flags=tmpflags;

    x->exp-=exponent;

    if(!(y->flags&F_NEGATIVE)) x->flags^=F_NEGATIVE;

    add_real_mul(ynext,y,x,2);  // y(i+1)=y(i)+S(i)*x(i)

    decRReg[4].flags=y->flags&F_NEGATIVE;

    add_real(znext,z,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);


    // ITERATION WITH 1

    tmpexp=ynext->exp;
    tmpflags=ynext->flags;
    ynext->exp-=exponent;
    ynext->flags=F_NEGATIVE;
    add_real(x,xnext,ynext);  // x(i+1)=x(i)+S(i)*y(i)

    ynext->exp=tmpexp;
    ynext->flags=tmpflags;
    xnext->exp-=exponent;
    if(!(ynext->flags&F_NEGATIVE)) xnext->flags^=F_NEGATIVE;
    add_real(y,ynext,xnext);  // x(i+1)=x(i)+S(i)*y(i)

    atanh_1_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=ynext->flags&F_NEGATIVE;

    add_real(z,znext,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(x);
    normalize(y);
    normalize(z);

++startexp;
}



for(exponent=startexp;exponent<startexp+digits;++exponent)
{
    // ITERATION W/5

    // decRReg[3]= (5*10^-exponent)*y

    tmpflags=y->flags;
    tmpexp=y->exp;
    y->exp-=exponent;

    //if(!(y->flags&F_NEGATIVE)) decRReg[3].flags^=F_NEGATIVE;
    y->flags=F_NEGATIVE;
    add_real_mul(xnext,x,y,5); // x(i+1)=x(i)+S(i)*y(i)

    y->exp=tmpexp;
    y->flags=tmpflags;

    x->exp-=exponent;

    if(!(y->flags&F_NEGATIVE)) x->flags^=F_NEGATIVE;

    add_real_mul(ynext,y,x,5);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_5_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=y->flags&F_NEGATIVE;

    add_real(znext,z,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);

    // FIRST ITERATION WITH 2

    // decRReg[3]= (2*10^-exponent)*y

    tmpflags=ynext->flags;
    tmpexp=ynext->exp;
    ynext->exp-=exponent;

    //if(!(y->flags&F_NEGATIVE)) decRReg[3].flags^=F_NEGATIVE;
    ynext->flags=F_NEGATIVE;
    add_real_mul(x,xnext,ynext,2); // x(i+1)=x(i)+S(i)*y(i)

    ynext->exp=tmpexp;
    ynext->flags=tmpflags;

    xnext->exp-=exponent;

    if(!(ynext->flags&F_NEGATIVE)) xnext->flags^=F_NEGATIVE;

    add_real_mul(y,ynext,xnext,2);  // y(i+1)=y(i)+S(i)*x(i)

    atanh_2_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=ynext->flags&F_NEGATIVE;

    add_real(z,znext,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(x);
    normalize(y);
    normalize(z);


    // SECOND ITERATION WITH 2


    tmpflags=y->flags;
    tmpexp=y->exp;
    y->exp-=exponent;

    //if(!(y->flags&F_NEGATIVE)) decRReg[3].flags^=F_NEGATIVE;
    y->flags=F_NEGATIVE;
    add_real_mul(xnext,x,y,2); // x(i+1)=x(i)+S(i)*y(i)

    y->exp=tmpexp;
    y->flags=tmpflags;

    x->exp-=exponent;

    if(!(y->flags&F_NEGATIVE)) x->flags^=F_NEGATIVE;

    add_real_mul(ynext,y,x,2);  // y(i+1)=y(i)+S(i)*x(i)

    decRReg[4].flags=y->flags&F_NEGATIVE;

    add_real(znext,z,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(xnext);
    normalize(ynext);
    normalize(znext);


    // ITERATION WITH 1

    tmpexp=ynext->exp;
    tmpflags=ynext->flags;
    ynext->exp-=exponent;
    ynext->flags=F_NEGATIVE;
    add_real(x,xnext,ynext);  // x(i+1)=x(i)+S(i)*y(i)

    ynext->exp=tmpexp;
    ynext->flags=tmpflags;
    xnext->exp-=exponent;
    if(!(ynext->flags&F_NEGATIVE)) xnext->flags^=F_NEGATIVE;
    add_real(y,ynext,xnext);  // x(i+1)=x(i)+S(i)*y(i)

    atanh_1_table(exponent,&decRReg[4]);     // GET Alpha(i)
    decRReg[4].flags=ynext->flags&F_NEGATIVE;

    add_real(z,znext,&decRReg[4]);  // z(i+1)=z(i)-Alpha(i)

    normalize(x);
    normalize(y);
    normalize(z);




}
// THE FINAL RESULTS ARE ALWAYS IN RREG[0], RREG[1] AND RREG[2]

// FINAL ROTATION SHOULD NOT AFFECT THE Kh CONSTANT
//mpd_qfma(xnext,z,y,x,&status);  // x(i+1)=x(i)+S(i)*y(i)
//mpd_qfma(ynext,z,x,y,&status);  // y(i+1)=y(i)+S(i)*x(i)

// THE FINAL RESULTS ARE ALWAYS IN RREG[6] AND RREG[7]

// RESULTS HAVE TYPICALLY 9 DIGITS MORE THAN REQUIRED, NONE OF THEM ARE ACCURATE
// SO ROUNDING/FINALIZING IS NEEDED
}


// CALCULATES ATANH(x0), AND RETURNS IT IN RREG[0]

void dechyp_atanh(REAL *x0)
{
// THE ONLY REQUIREMENT IS THAT y0 <= x0
int negx=x0->flags&F_NEGATIVE;
int startexp;
REAL one;

x0->flags^=negx;
// ALWAYS: NEED TO WORK ON PRECISION MULTIPLE OF 9
Context.precdigits+=8;

startexp=-x0->exp-((x0->len-1)<<3)-sig_digits(x0->data[x0->len-1]);

if(startexp<1) {

decconst_One(&one);


// CRITERIA FOR REPETITION OF INITIAL STEP
// REQUIRED IN ORDER TO INCREASE THE RANGE OF CONVERGENCE
sub_real(&decRReg[3],&one,x0);
normalize(&decRReg[3]);
startexp=(decRReg[3].exp+((decRReg[3].len-1)<<3)+sig_digits(decRReg[3].data[decRReg[3].len-1]))*2;

}

    copyReal(&decRReg[2],x0);

    decRReg[1].len=1;
    decRReg[1].data[0]=1;
    decRReg[1].exp=0;
    decRReg[1].flags=0;

 // USE CORDIC TO COMPUTE
    // z = 0
    decRReg[0].len=1;
    decRReg[0].data[0]=0;
    decRReg[0].exp=0;
    decRReg[0].flags=0;


    CORDIC_Hyp_Vectoring_unrolled((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,startexp);


if(negx) decRReg[0].flags|=F_NEGATIVE;

Context.precdigits-=8;

// HERE decRReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}


// CALCULATES LN(x0), AND RETURNS IT IN RREG[0]
// ARGUMENT MUST BE POSITIVE, NO ARGUMENT CHECKS HERE

void dechyp_ln(REAL *x0)
{
    int adjustexp;
    int startexp=1;
REAL ln10,one;

Context.precdigits+=8;

decconst_ln10(&ln10);
decconst_One(&one);

copyReal(&decRReg[3],x0);
adjustexp=x0->exp+( ((x0->len-1)<<3)+sig_digits(x0->data[x0->len-1])-1);
decRReg[3].exp=-(((decRReg[3].len-1)<<3)+sig_digits(decRReg[3].data[decRReg[3].len-1])-1);    // TAKE ONLY THE MANTISSA, LEFT JUSTIFIED

// y0=A-1
sub_real(&decRReg[2],&decRReg[3],&one);
// x0=A+1
add_real(&decRReg[1],&decRReg[3],&one);
normalize(&decRReg[2]);
normalize(&decRReg[1]);

// z = 0
decRReg[0].len=1;
decRReg[0].data[0]=0;
decRReg[0].exp=0;
decRReg[0].flags=0;


CORDIC_Hyp_Vectoring_unrolled((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,startexp);

// ADD BACK THE EXPONENT AS LN(A)=EXP*LN(10)+LN(A')
int2real(&decRReg[4],adjustexp);
mul_real(&decRReg[4],&decRReg[4],&ln10);
normalize(&decRReg[4]);
add_real(&decRReg[3],&decRReg[0],&decRReg[0]);
add_real(&decRReg[0],&decRReg[3],&decRReg[4]);

Context.precdigits-=8;

// HERE decRReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}

// USE A CORDIC LOOP TO COMPUTE THE SQUARE ROOT
// SAME AS LN, BUT WE USE THE RESULT FROM x

void dechyp_sqrt(REAL *x0)
{
    int adjustexp;
    int startexp=1;

    REAL Kh1,one;
    Context.precdigits+=8;

    decconst_One(&one);
/*
    startexp=-x0->exp-((x0->len-1)<<3)-sig_digits(x0->data[x0->len-1]);

    if(startexp<1) {

    // CRITERIA FOR REPETITION OF INITIAL STEP
    // REQUIRED IN ORDER TO INCREASE THE RANGE OF CONVERGENCE
    mpd_sub(&decRReg[3],&one,x0);

    startexp=(decRReg[3].exp+decRReg[3].digits)*2;

    }
*/

    if(iszeroReal(x0)) {

     decRReg[0].data[0]=0;
     decRReg[0].len=1;
     decRReg[0].exp=0;
     decRReg[0].flags=0;

     Context.precdigits-=8;

     return;
    }


copyReal(&decRReg[3],x0);
adjustexp=x0->exp+( ((x0->len-1)<<3)+sig_digits(x0->data[x0->len-1])-1);
decRReg[3].exp=-(((decRReg[3].len-1)<<3)+sig_digits(decRReg[3].data[decRReg[3].len-1])-1);    // TAKE ONLY THE MANTISSA, LEFT JUSTIFIED


if(adjustexp&1) {
    // MAKE IT AN EVEN EXPONENT, SO IT'S EASY TO DIVIDE BY 2
    --decRReg[3].exp;
    adjustexp+=1;
}

// y0=A-1
sub_real(&decRReg[2],&decRReg[3],&one);
// x0=A+1
add_real(&decRReg[1],&decRReg[3],&one);
normalize(&decRReg[2]);
normalize(&decRReg[1]);


// z = 0
decRReg[0].len=1;
decRReg[0].data[0]=0;
decRReg[0].exp=0;
decRReg[0].flags=0;


CORDIC_Hyp_Vectoring_unrolled((Context.precdigits>REAL_PRECISION_MAX)? REAL_PRECISION_MAX+8:Context.precdigits,startexp);

decconst_Kh1(&Kh1);

// ADD BACK THE EXPONENT AS sqrt(A)= 2*sqrt(xin^2-yin^2) * Kh1 * 10^(exponent/2)
int2real(&decRReg[3],5);
normalize(&decRReg[1]);
mul_real(&decRReg[4],&decRReg[3],&decRReg[1]);
decRReg[4].exp--;
normalize(&decRReg[4]);
mul_real(&decRReg[0],&decRReg[4],&Kh1);
decRReg[0].exp+=adjustexp/2;

Context.precdigits-=8;

// HERE decRReg[0] CONTAINS THE ANGLE WITH 9 DIGITS MORE THAN THE CURRENT PRECISION (NONE OF THEM WILL BE ACCURATE), ROUNDING IS REQUIRED
// THE ANGLE IS IN THE RANGE -PI, +PI
// THE LAST DIGIT MIGHT BE OFF BY +/-1 WHEN USING THE MAXIMUM SYSTEM PRECISION

}


void dechyp_asinh(REAL *x)
{
    REAL one;
    decconst_One(&one);

    Context.precdigits+=8;

    mul_real(&decRReg[1],x,x);   // 1 = x^2

    add_real(&decRReg[7],&decRReg[1],&one);   // 2 = x^2+1

    normalize(&decRReg[7]);
    Context.precdigits-=8;

    dechyp_sqrt(&decRReg[7]); // 7 = cosh = sqrt(sinh^2+1)

    Context.precdigits+=8;
    normalize(&decRReg[0]);

    add_real(&decRReg[2],&decRReg[0],&one);   // 2 = cosh + 1
    normalize(&decRReg[2]);

    div_real(&decRReg[7],x,&decRReg[2],Context.precdigits); // 7 = sinh / (cosh + 1)
    normalize(&decRReg[7]);
    Context.precdigits-=8;


    dechyp_atanh(&decRReg[7]);

    add_real(&decRReg[1],&decRReg[0],&decRReg[0]);

}

void dechyp_acosh(REAL *x)
{
    REAL one;
    decconst_One(&one);

    Context.precdigits+=8;

    sub_real(&decRReg[1],x,&one);   // 1 = x-1
    add_real(&decRReg[2],x,&one);   // 2 = x+1
    normalize(&decRReg[1]);
    normalize(&decRReg[2]);
    div_real(&decRReg[7],&decRReg[1],&decRReg[2],Context.precdigits);
    normalize(&decRReg[7]);
    Context.precdigits-=8;

    dechyp_sqrt(&decRReg[7]);

    normalize(&decRReg[0]);

    dechyp_atanh(&decRReg[0]);

    add_real(&decRReg[1],&decRReg[0],&decRReg[0]);

}

