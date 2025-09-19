#ifndef MATCHING_H
#define MATCHING_H

class Matching
{
public:

	Matching();
	~Matching();

	void match_images();

private:

	void match_images_with_pairs();
	void save_matches();
	void match();
	void _match_robust_impl();
	void match_robust();
	void match_words();
	void match_words_symmetric();
};

#endif // #ifndef MATCHING_H
