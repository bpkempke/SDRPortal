
/*
 * hierarchicalDataflowBlock implements the necessary functionality to connect separate independent classes together to
 *   provide a generic RX/TX dataflow path between each class in the hierarchy
 */
class hierarchicalDataflowBlock{
public:
	hierarchicalDataflowBlock(int num_down_channels=1, int num_up_channels=1);
	void addUpperLevel(hierarchicalDataflowBlock *in_block, int up_channel=0, int local_up_channel=0);
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0) = 0;
	void dataToUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	void addLowerLevel(hierarchicalDataflowBlock *in_block, int down_channel=0, int local_down_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0) = 0;
	void dataToLowerLevel(void *data, int num_bytes, int local_down_channel=0);
	void removeUpperLevel(hierarchicalDataflowBlock *in_block);
	void removeLowerLevel(hierarchicalDatafloweBlock *in_block);
private:
	struct hierarchicalDataConnection {
		int local_channel;
		int remote_channel;
		hierarchicalDataflowBlock *remote;
	};

	std::vector<std::vector<hierarchicalDataConnection> > upper_level_links;
	std::vector<std::vector<hierarchicalDataConnection> > lower_level_links;
};
