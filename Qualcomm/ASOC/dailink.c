
// DAI link定义
// ref: kernel/msm-5.4/techpack/audio/asoc/sa6155.c
// ref: kernel/msm-5.4/Documentation/sound/soc/dpcm.rst，注意此文件中的代码用例与实际的已经不符.

// 在高通平台dailink 分为FE dailink和BE dailink，
// FE dailink是连接CPU和DSP的dai连接，实际上高通在cpu和dsp之间是sharemem的方式传输数据，所以FE dailink的codec dai并不存在，所以codec dai的名字用dummy
// BE dailink是连接DSP和codec的dai连接，我们的项目中codec driver没按照高通的方式，所以BE dailink中的codec dai名字用stub占位
/*
FE and BE DAI links
-------------------
::

  | Front End PCMs    |  SoC DSP  | Back End DAIs | Audio devices |
  
                      *************
  PCM0 <------------> *           * <----DAI0-----> Codec Headset
                      *           *
  PCM1 <------------> *           * <----DAI1-----> Codec Speakers
                      *   DSP     *
  PCM2 <------------> *           * <----DAI2-----> MODEM
                      *           *
  PCM3 <------------> *           * <----DAI3-----> BT
                      *           *
                      *           * <----DAI4-----> DMIC
                      *           *
                      *           * <----DAI5-----> FM
                      *************
*/

//FE dailink
/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link msm_common_dai_links[] = {
	/* FrontEnd DAI Links */
	{
		.name = MSM_DAILINK_NAME(Media1),  //"SA6155 Media1"
		.stream_name = "MultiMedia1",  //与pcm设备关联
		.dynamic = 1,    //表示这个dai link在运行时可以动态路由到其他的dai link
#if IS_ENABLED(CONFIG_AUDIO_QGKI)
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,  //4
#endif /* CONFIG_AUDIO_QGKI */
		.dpcm_playback = 1,  
		.dpcm_capture = 1,  
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST}, //这里指trigger DSP是在其他组件之前还是之后
		.ignore_suspend = 1,  //在suspend时保持dai为激活状态
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,  // pmdown_time is ignored at stop
		.id = MSM_FRONTEND_DAI_MULTIMEDIA1,  //0 machine driver的link id
		SND_SOC_DAILINK_REG(multimedia1),    // 宏，定义cpu codec platform
	},
	// ...
}

static struct snd_soc_dai_link msm_custom_fe_dai_links[] = {
	/* FrontEnd DAI Links */
	{
		.name = MSM_DAILINK_NAME(Media1),
		.stream_name = "MultiMedia1",  
		.dynamic = 1,  
#if IS_ENABLED(CONFIG_AUDIO_QGKI)
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
#endif /* CONFIG_AUDIO_QGKI */
		.dpcm_playback = 1,   
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
				SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = MSM_FRONTEND_DAI_MULTIMEDIA1,
		.ops = &msm_fe_qos_ops,
		SND_SOC_DAILINK_REG(multimedia1),
	},
	// ...
}

SND_SOC_DAILINK_REG(multimedia1) 展开后包含：
.cpus = multimedia1_cpus, 
.codecs = multimedia1_codecs,
.platforms = multimedia1_platforms,

// kernel/msm-5.4/techpack/audio/asoc/msm_dailink.h
#define SND_SOC_DAILINK_DEF(name, def...)		\
	static struct snd_soc_dai_link_component name[]	= { def }

/* FE dai-links */
SND_SOC_DAILINK_DEFS(multimedia1,
	DAILINK_COMP_ARRAY(COMP_CPU("MultiMedia1")),
	DAILINK_COMP_ARRAY(COMP_CODEC("snd-soc-dummy", "snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-dsp.0")));

SND_SOC_DAILINK_DEF(multimedia1_cpus, { .dai_name = "MultiMedia1", }); 
SND_SOC_DAILINK_DEF(multimedia1_codecs, { .name = "snd-soc-dummy", .dai_name = "snd-soc-dummy-dai", }); 
SND_SOC_DAILINK_DEF(multimedia1_platforms, { .name = "msm-pcm-dsp.0" })
//分别是cpu组件，驱动在msm-dai-fe.c；codec组件；平台组件，驱动在 msm-pcm-q6-v2.c
//dai link的三个要素:
//cpu dai: 这里是fe dailink的cpu dai，cpu与adsp是share mem方式，并没有实际的物理dai接口,这里的MultiMedia1与pcm设备对应
//codec dai: 用dummy，因为BE是动态的，fe dailink没有实际的codec dai.
//platform：提供数据的搬移功能，这里有3种参数，0代表普通模式，1代表ll(low latency)模式，2代表ull模式.


// FE DAI 定义
// kernel/msm-5.4/techpack/audio/asoc/msm-dai-fe.c
// 这些dai links会在machine驱动probe的时候，将dai links信息给声卡 card->dai_link ，声卡注册的时候，会根据这些信息创建相应的pcm设备
static struct snd_soc_dai_driver msm_fe_dais[] = {
	{
		.playback = {
			.stream_name = "MultiMedia1 Playback",
			.aif_name = "MM_DL1",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.capture = {
			.stream_name = "MultiMedia1 Capture",
			.aif_name = "MM_UL1",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.ops = &msm_fe_Multimedia_dai_ops, // 目前就只有startup方法
		.name = "MultiMedia1",  // 与dai link里cpu组件名字相同，匹配上
		.probe = fe_dai_probe,  
	},
	// ...
}




// BE DAI link
// kernel/msm-5.4/techpack/audio/asoc/sa6155.c
// common 只定义了RX0和TX0， 其他的定义在auto中
static struct snd_soc_dai_link msm_common_be_dai_links[] = {
	/* Backend AFE DAI Links */
	{
		.name = LPASS_BE_AFE_PCM_RX,  //"RT_PROXY_DAI_001_RX"
		.stream_name = "AFE Playback",
		.no_pcm = 1,  //be dailink不会创建pcm逻辑设备，FE dailink才会创建
		.dpcm_playback = 1,
		.id = MSM_BACKEND_DAI_AFE_PCM_RX,   //9
		.be_hw_params_fixup = msm_be_hw_params_fixup,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(afe_pcm_rx),
	},
	// ...
	{
		.name = LPASS_BE_PRI_TDM_RX_0,  //PRI_TDM_RX_0
		.stream_name = "Primary TDM0 Playback",
		.no_pcm = 1,  //be dailink不会创建pcm逻辑设备，FE dailink才会创建
		.dpcm_playback = 1,  //动态pcm
		.id = MSM_BACKEND_DAI_PRI_TDM_RX_0,  //55
		.be_hw_params_fixup = msm_tdm_be_hw_params_fixup,  //此处是machine driver按照FE hw params重新配置dai
		.ops = &sa6155_tdm_be_ops,
#ifndef CONFIG_SND_SUPPORT_AUDIO_S2R
		.ignore_suspend = 1, //
#endif
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(pri_tdm_rx_0),
	},
	// ...
}

static struct snd_soc_dai_link msm_auto_be_dai_links[] = {
	/* Backend DAI Links */
	{
		.name = LPASS_BE_PRI_TDM_RX_1,   //"PRI_TDM_RX_1"
		.stream_name = "Primary TDM1 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = MSM_BACKEND_DAI_PRI_TDM_RX_1,
		.be_hw_params_fixup = msm_tdm_be_hw_params_fixup,
		.ops = &sa6155_tdm_be_ops,
#ifndef CONFIG_SND_SUPPORT_AUDIO_S2R
		.ignore_suspend = 1,
#endif
		SND_SOC_DAILINK_REG(pri_tdm_rx_1),
	},
	// ...
}
SND_SOC_DAILINK_REG(pri_tdm_rx_1),展开包含：
.cpus = pri_tdm_rx_1_cpus,
.codecs = pri_tdm_rx_1_codecs,
.platforms = pri_tdm_rx_1_platforms,

SND_SOC_DAILINK_DEFS(pri_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36866")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEF(pri_tdm_rx_1_cpus, { .dai_name = "msm-dai-q6-tdm.36866", });  //be dai的名字
SND_SOC_DAILINK_DEF(pri_tdm_rx_1_codecs, { .name = "msm-stub-codec.1", .dai_name = "msm-stub-rx", }); //这里的stub是占位的意思，没有实际的codec
SND_SOC_DAILINK_DEF(pri_tdm_rx_1_platforms, { .name = "msm-pcm-routing" }) 
//dai link的三个要素:
//cpu dai: 这里是be dailink的cpu dai，实际是dsp的dai，这里有实际的dai接口，36866代表tdm口的逻辑port
//codec dai: 用stub，是占位的意思，因为我们的项目中codec是由外部管理的。在高通的demo项目中有用到实际的codec，就有实际的codec name
//platform：提供pcm的路由功能。

// BE DAI
// kernel/msm-5.4/techpack/audio/asoc/msm-dai-q6-v2.c
static struct snd_soc_dai_driver msm_dai_q6_tdm_dai[] = {
	{
		.playback = {
			.stream_name = "Primary TDM0 Playback",
			.aif_name = "PRI_TDM_RX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX,
		.probe = msm_dai_q6_dai_tdm_probe,
		.remove = msm_dai_q6_dai_tdm_remove,
	},
	// ...
	{
		.capture = {
			.stream_name = "Primary TDM0 Capture",
			.aif_name = "PRI_TDM_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX,
		.probe = msm_dai_q6_dai_tdm_probe,
		.remove = msm_dai_q6_dai_tdm_remove,
	},
	// ...
}	


//Hostless PCM streams
//有两种形式，一种是CODEC <-> CODEC 形式，
//第二种是Hostless FE，这个是用来建立一个虚拟的path，不能读写数据


