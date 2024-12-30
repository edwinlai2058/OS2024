# Copyright (C) 2021 justin0u0<mail@justin0u0.com>

import click

@click.command()
@click.option('--output', default='./tests/00.ans', help='Output file path.')
@click.option('--answer', default='./tests/00.out', help='Answer file path.')
def verify(output, answer):
	with open(output, 'r') as output_f, open(answer, 'r') as answer_f:
		output_lines = sorted(output_f.readlines())
		answer_lines = sorted(answer_f.readlines())

		if output_lines != answer_lines:
			print('\n\033[1;31;48m' + f'fail QAQ.' + '\033[1;37;0m')
		else:
			print('\n\033[1;32;48m' + f'success ouo.' + '\033[1;37;0m')

if __name__ == '__main__':
	verify()
