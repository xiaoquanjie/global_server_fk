
class LinuxSockBase
{
protected:
	struct data
	{
		int _init_cnt;
	};
};

template<s_int32_t Version=1>
class LinuxSockInit : public LinuxSockBase
{
public:
	M_SOCKET_DECL LinuxSockInit();

	M_SOCKET_DECL ~LinuxSockInit();

protected:
	static data _data;
};

template<s_int32_t Version>
LinuxSockBase::data LinuxSockInit<Version>::_data;

template<s_int32_t Version>
M_SOCKET_DECL LinuxSockInit<Version>::LinuxSockInit()
{
	if (::__sync_add_and_fetch(&_data._init_cnt,1)==1){
		signal(SIGPIPE, SIG_IGN);
	}
}

template<s_int32_t Version>
M_SOCKET_DECL LinuxSockInit<Version>::~LinuxSockInit()
{
	if (::__sync_sub_and_fetch(&_data._init_cnt, 1) == 0){
	}
}

static const LinuxSockInit<>& gLinuxSockInstance = LinuxSockInit<>();
