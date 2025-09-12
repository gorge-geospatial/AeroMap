#ifndef TRACKING_H
#define TRACKING_H

class Tracking
{
public:

	Tracking();
	~Tracking();

private:

	void load_fetures();
	void load_matches();
	void common_tracks();
	void all_common_tracks_with_features();
	void all_common_tracks_without_features();
	void all_common_tracks();
	void np_all_common_tracks_with_features();
	void as_weighted_graph();
	void as_graph();
	void create_tracks_manager();

	bool _good_tracks();
};

#endif // #ifndef TRACKING_H
