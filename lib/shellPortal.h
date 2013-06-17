struct cmdListenerArgs{
	shellPortal *shell_portal_ptr;
	int down_channel;
	FILE *command_fp;
	pthread_t *thread_ptr;
};

