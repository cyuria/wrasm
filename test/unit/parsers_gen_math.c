
/* TODO: add tests for math shortcut parsers */

int main(void)
{
	struct parser_t mv;
	struct parser_t not ;
	struct parser_t neg;
	struct parser_t negw;
	struct parser_t sextw;
	if (parse_parser("mv", &mv))
		return 1;
	if (parse_parser("not", &not ))
		return 1;
	if (parse_parser("neg", &neg))
		return 1;
	if (parse_parser("negw", &negw))
		return 1;
	if (parse_parser("sext.w", &sextw))
		return 1;
}
