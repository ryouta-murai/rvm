#include <stdio.h>

/*
 ****************************************************************
 *
 ****************************************************************
 */

typedef unsigned char apm_base_t;
typedef unsigned long apm_super_t;

#define APM_BASE_ZERO		((apm_base_t) 0)
#define APM_BASE_ONE		((apm_base_t) 1)
#define APM_BASE_MAX		((apm_base_t) -1)
#define APM_BASE_BIT_SIZE	(8 * sizeof(apm_base_t))
#define APM_BASE_BIT_HALF_SIZE	(APM_BASE_BIT_SIZE / 2)
#define APM_BASE_FULL		(APM_BASE_MAX)
#define APM_BASE_HALF_FULL	(APM_BASE_FULL >> APM_BASE_BIT_HALF_SIZE)

/*
 ****************************************************************
 *
 ****************************************************************
 */

apm_base_t apm_base_increment(
	apm_base_t *result,
	apm_base_t operand)
{
	*result = operand + APM_BASE_ONE;

	if(*result == APM_BASE_ZERO) {
		return APM_BASE_ONE;
	}

	return APM_BASE_ZERO;
}

apm_base_t apm_base_shift(
	apm_base_t *result,
	apm_base_t operand)
{
	*result = operand << 1;

	return operand >> (APM_BASE_BIT_SIZE - 1);
}

/*
 ****************************************************************
 *
 ****************************************************************
 */

apm_base_t apm_base_add(
	apm_base_t *result,
	apm_base_t operand1,
	apm_base_t operand2,
	apm_base_t carry_in)
{
	apm_base_t carry_out;

	*result = operand1 + operand2 + carry_in;

	if(carry_in == APM_BASE_ZERO) {
/* when carry_in is zero */
		if(*result < operand1 || *result < operand2) {
			return APM_BASE_ONE;
		} else {
			return APM_BASE_ZERO;
		}
	} else {
/* when carry_in is one */
		if(*result <= operand1 || *result <= operand2) {
			return APM_BASE_ONE;
		} else {
			return APM_BASE_ZERO;
		}
	}

}

/*
 ****************************************************************
 *
 ****************************************************************
 */

void apm_base_array_zero(
	apm_base_t *array,
	int length)
{
	int i;

	for(i = 0; i < length; ++i) {
		array[i] = APM_BASE_ZERO;
	}
}

void apm_base_array_copy(
	apm_base_t *to,
	apm_base_t *from,
	int length)
{
	int i;

	for(i = 0; i < length; ++i) {
		to[i] = from[i];
	}
}

void apm_base_array_negate(
	apm_base_t *to,
	apm_base_t *from,
	int length)
{
	int i;

	for(i = 0; i < length; ++i) {
		to[i] = ~from[i];
	}
}

/*
 ****************************************************************
 *
 ****************************************************************
 */

apm_base_t apm_base_array_increment(
	apm_base_t *operand,
	int operand_length)
{
	int i;
	apm_base_t carry;

	carry = APM_BASE_ONE;
	for(i = 0; i < operand_length && carry != APM_BASE_ZERO; ++i) {
		carry = apm_base_increment(
			&operand[i],
			operand[i]);
	}

	return carry;
}
apm_base_t apm_base_array_shift(
	apm_base_t *operand,
	int operand_length)
{
	int i;
	apm_base_t carry;

	carry = APM_BASE_ZERO;
	for(i = 0; i < operand_length && carry != APM_BASE_ZERO; ++i) {
		carry = apm_base_shift(
			&operand[i],
			operand[i] | carry);
	}

	return carry;
}
/*
 ****************************************************************
 *
 ****************************************************************
 */

apm_base_t apm_base_array_add(
	apm_base_t *short_operand,
	int short_operand_length,
	apm_base_t *long_operand,
	int long_operand_length,
	apm_base_t carry_in)
{
	int i;
	apm_base_t carry;

	carry = carry_in;
	for(i = 0; i < short_operand_length; ++i) {
		carry = apm_base_add(
			&long_operand[i],
			short_operand[i],
			long_operand[i],
			carry);
	}
	if(carry != APM_BASE_ZERO) {
		carry = apm_base_array_increment(
			&long_operand[short_operand_length],
			long_operand_length - short_operand_length);
	}

	return carry;
}

void apm_base_array_negative(
	apm_base_t *short_operand,
	int short_operand_length,
	apm_base_t *long_operand,
	int long_operand_length)
{
	apm_base_array_zero(
		long_operand,
		long_operand_length);
	apm_base_array_copy(
		long_operand,
		short_operand,
		short_operand_length);
	apm_base_array_negate(
		long_operand,
		long_operand,
		long_operand_length);
	apm_base_array_increment(
		long_operand,
		long_operand_length);
}

/*
 ****************************************************************
 *
 ****************************************************************
 */

void apm_base_multiply(
	apm_base_t *lower,
	apm_base_t *upper,
	apm_base_t operand1,
	apm_base_t operand2)
{
	apm_super_t temp;

	temp = (apm_super_t)operand1 * (apm_super_t)operand2;
	*lower = temp & APM_BASE_FULL;
	*upper = temp >> APM_BASE_BIT_SIZE;

	return;
}

/*
 ****************************************************************
 *
 ****************************************************************
 */

void apm_base_array_multiply(
	apm_base_t *result,
	apm_base_t *short_operand,
	int short_operand_length,
	apm_base_t *long_operand,
	int long_operand_length)
{
	int i, j;
	int result_length;
	apm_base_t carry;

	result_length = short_operand_length + long_operand_length;
	apm_base_array_zero(
		result,
		result_length);

	for(i = 0; i < short_operand_length; ++i) {
		for(j = 0; j < long_operand_length; ++j) {
			apm_base_t temp[2];

			apm_base_multiply(
				&temp[0],
				&temp[1],
				short_operand[i],
				long_operand[j]);
			apm_base_array_add(
				temp,
				2,
				&result[i + j],
				result_length - (i + j),
				APM_BASE_ZERO);
		}
	}
}

void apm_base_array_inverse(
	apm_base_t *operand,
	int operand_length,
	apm_base_t *short_operand,
	int short_operand_length,
	apm_base_t *short_operand2,
	int short_operand_length2,
	apm_base_t *long_operand,
	int long_operand_length)
{
	apm_base_t two;

	two = APM_BASE_ONE + APM_BASE_ONE;

	apm_base_array_zero(
		long_operand,
		long_operand_length);

	apm_base_array_multiply(
		short_operand2,
		short_operand,
		short_operand_length,
		short_operand,
		short_operand_length);
	apm_base_array_multiply(
		long_operand,
		operand,
		operand_length,
		short_operand2,
		short_operand_length2);
	apm_base_array_negative(
		long_operand,
		long_operand_length,
		long_operand,
		long_operand_length);

#if 0
	apm_base_array_add(
		short_operand,
		short_operand_length,
		&long_operand[long_operand_length - short_operand_length],
		long_operand_length - short_operand_length,
		APM_BASE_ZERO);
	apm_base_array_add(
		short_operand,
		short_operand_length,
		&long_operand[long_operand_length - short_operand_length],
		long_operand_length - short_operand_length,
		APM_BASE_ZERO);
#endif
}

/*
 ****************************************************************
 *
 ****************************************************************
 */


void print_number(
	apm_base_t *array,
	int length)
{
	int i;

	for(i = 0; i < length; ++i) {
		printf("%02x ", array[i]);
	}
	printf("(%u)\n", *(unsigned int *)array);
}

#define NUMBER	(10)
#define LENGTH	(sizeof(int))
apm_base_t number[NUMBER][LENGTH * 2];

void add_test()
{
	unsigned int i, j;
	apm_base_t to[sizeof(int)];
	apm_base_t from[sizeof(int)];

	for(i = 0; i < 0x10000; ++i) {
		if(i % (0x10000 / 100) == 0) {
			printf("%d/100 finished\n",
				i / (0x10000 / 100));
		}

		for(j = 0; j < 0x10000; ++j) {
			*(unsigned int *)to = i;
			*(unsigned int *)from = j;

			apm_base_array_add(
				from,
				sizeof(int),
				to,
				sizeof(int),
				APM_BASE_ZERO);
			if(*(int *)to != i + j) {
				printf("# ERROR i, j = %d, %d\n", i, j);
			}
		}
	}
}
void subtract_test()
{
	unsigned int i, j;
	apm_base_t to[sizeof(int)];
	apm_base_t from[sizeof(int)];
	apm_base_t neg[sizeof(int)];

	for(i = 0; i < 0x10000; ++i) {
		if(i % (0x10000 / 100) == 0) {
			printf("%d/100 finished\n",
				i / (0x10000 / 100));
		}

		for(j = 0; j < i; ++j) {
			*(unsigned int *)to = i;
			*(unsigned int *)from = j;
			apm_base_array_negative(
				from,
				sizeof(int),
				neg,
				sizeof(int));
			apm_base_array_add(
				neg,
				sizeof(int),
				to,
				sizeof(int),
				APM_BASE_ZERO);
			if(*(int *)to != i - j) {
				printf("# ERROR i, j = %d, %d\n", i, j);
			}
		}
	}
}
void multiply_test()
{
	unsigned int i, j;
	apm_base_t num1[sizeof(int)];
	apm_base_t num2[sizeof(int)];
	apm_base_t res[sizeof(int) * 2];

	for(i = 0; i < 0x10000; ++i) {
		if(i % (0x10000 / 100) == 0) {
			printf("%d/100 finished\n",
				i / (0x10000 / 100));
		}

		for(j = 0; j < 0x10000; ++j) {
			*(unsigned int *)num1 = i;
			*(unsigned int *)num2 = j;

			apm_base_array_multiply(
				res,
				num1,
				sizeof(int),
				num2,
				sizeof(int));
			if(*(int *)res != i * j) {
				printf("# ERROR i, j = %d, %d\n", i, j);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	printf("hello, world\n");

	apm_base_array_zero(number[0], LENGTH);
	apm_base_array_zero(number[1], LENGTH);

	*(int *)number[0] = 100;
	*(int *)number[1] = 100;

	printf("################\n");
	printf("# test add\n");
//	add_test();
	printf("################\n");
	printf("# test subtract\n");
//	subtract_test();
	printf("################\n");
	printf("# test multiply\n");
//	multiply_test();

	apm_base_array_inverse(
		number[0],
		1,
		number[1],
		1,
		number[2],
		2,
		number[3],
		4);

	print_number(
		number[0],
		LENGTH);
	print_number(
		number[1],
		LENGTH);
	print_number(
		number[2],
		LENGTH);
	print_number(
		number[3],
		LENGTH);
}
