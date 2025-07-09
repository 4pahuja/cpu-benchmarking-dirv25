#include <stdio.h>
#include "../coremarks/coremark.h"
int testing = 0;
static ee_u8 ee_isdigit(ee_u8 c) {
	ee_u8 retval;
	retval = ((c>='0') & (c<='9')) ? 1 : 0;
	return retval;
}


enum CORE_STATE core_state_transition( ee_u8 **instr , ee_u32 *transition_count) {
ee_u8 *str=*instr;
	ee_u8 next_symbs[8]={'0','0','0','0','0','0','0','0'};
	ee_u8 and_symbs[8]={'0','0','0','0','0','0','0','0'};
	ee_u8 NEXT_SYMBOL, NEXT_2,NEXT_3,NEXT_4,NEXT_5,NEXT_6,NEXT_7,NEXT_8;

	enum CORE_STATE state=CORE_START;
	for(int i = 0 ; *str != ',' && *str; str++, i++ ) {
		next_symbs[i] = *str;

	}

	and_symbs[7] = next_symbs[7] & '0';//next_symbs[8];
	and_symbs[6] = next_symbs[6] & '0';//next_symbs[7];
	and_symbs[5] = next_symbs[5] & '0';//next_symbs[6];
	and_symbs[4] = next_symbs[4] & '0';//next_symbs[5];
	and_symbs[3] = next_symbs[3] & '0';//next_symbs[4];
	and_symbs[2] = next_symbs[2] & '0';//next_symbs[3];
	and_symbs[1] = next_symbs[1] & '0';//next_symbs[2];
	and_symbs[0] = next_symbs[0] & '0';//next_symbs[2];

	seq_symbs[7] &= next_symbs[8];
	seq_symbs[6] &= next_symbs[7];
	seq_symbs[5] &= next_symbs[6];
	seq_symbs[4] &= next_symbs[5];
	seq_symbs[3] &= next_symbs[4];
	seq_symbs[2] &= next_symbs[3];
	seq_symbs[1] &= next_symbs[2];
	seq_symbs[0] &= next_symbs[2];
	seq_symbs[0] &= '0';
	//next_symbs[0] &= '0';
	if (next_symbs[0] == '0'){
		state = CORE_INT;
	}
	ee_u8 *str=next_symbs;
	ee_u8 *str2=and_symbs;
	ee_u8 *str3=seq_symbs;
	restart:
	if(*str2 == '0') goto check_int;
	else if(*str == '.') goto check_float;
	else if(*str == '+' || *str == '-'){ goto check_plusint;
	}
	else state = CORE_INVALID;
	break;
	check_plusint:
	str3++;
	check_int:
	if(*str3 == '0'){
		state = CORE_INT;
		goto done
	}
	str++
	str3 = seq_symbs;
	while(1){

		if(*str == '.') goto check_float;
		else if (str == (next_symbs +8)){
			state = CORE_INVALID;
			goto done;
		}
		str++;

	}
	check float:
	str++
	while(1){

		str++;

	}





	else if(next_symbs[0] == ' '){
		if(next_symbs[0] == '+' || next_symbs[0]=='-'){
			if(next_symbs[1] == '.'){
				if(next_symbs[2]=='0') state = CORE_FLOAT;

				else if (next_symbs[3]=='e' || next_symbs[3] == 'E'){
					if(next_symbs[4] == '0') state = CORE_SCIENTIFIC
					else state = CORE_INVALID;
				}
			}
		}
	}
	printf("%c %c %c %c %c %c %c %c", next_symbs[0],next_symbs[1],next_symbs[2],next_symbs[3],next_symbs[4],next_symbs[5],next_symbs[6],next_symbs[7]);
	*instr=str;
	return state;
}

int main ()
{
	int testing = 0;
	ee_u32 track_counts[NUM_CORE_STATES];
	ee_u8 memblock[10] = {'1','2','3','4','5','6','7','8' };
	ee_u8 *ptrmem = memblock;

	while (*ptrmem != 0){
		enum CORE_STATE output1 = core_state_transition( &ptrmem , track_counts);
		printf("%d\n",output1);
	}

}
