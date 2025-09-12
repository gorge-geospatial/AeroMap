#ifndef PAIRSELECT_H
#define PAIRSELECT_H

class PairSelect
{
public:

	PairSelect();
	~PairSelect();

private:

	bool has_gps_info();
	void get_gps_point();
	double sign(double x);
	void get_gps_opk_point();
	double find_best_altitude();
	void get_representative_points();
	void match_candidates_by_distance();
	void match_candidates_by_graph();
	void match_candidates_with_bow();
	void compute_bow_affinity();
	void match_candidates_with_vlad();
	void cmpute_vlad_affinity();
	void preempt_candidates();
	void construct_pairs();
	void create_parallel_matching_args();
	void match_bow_unwrap_args();
	void match_vlad_unwrap_args();
	void match_candidates_by_time();
	void match_candidates_by_order();
	void match_candidates_from_metadata();
	void bow_distances();
	void load_histograms();
	void vlad_histogram_unwrap_args();
	void vlad_histograms();
	void pairs_from_neighbors();
	void ordered_pairs();
};

#endif // #ifndef PAIRSELECT_H
