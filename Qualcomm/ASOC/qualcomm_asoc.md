
# qualcomm asoc

***
### ASOC code path
ref: <https://segmentfault.com/a/1190000041093524>
- Platform-specific machine driver - /kernel/msm-5.4/techpack/audio/asoc/sa6155.c            // for sa6155 chipset
- MSM frontend CPU driver − /kernel/msm-5.4/techpack/audio/asoc/msm-dai-fe.c                 //MSM Frontend DAI driver
- Back-end CPU driver − /kernel/msm-5.4/techpack/audio/asoc/msm-dai-q6-v2.c                  //MSM DSP DAI driver（BE DAI）
- Stream routing platform driver − /kernel/msm-5.4/techpack/audio/asoc/msm-pcm-routing-v2.c  //MSM routing platform driver
- Playback and record platform driver − /kernel/msm-5.4/techpack/audio/asoc/msm-pcm-q6-v2.c  //PCM module platform driver
- Voice platform driver − kernel/msm-5.4/techpack/audio/asoc/msm-pcm-voice-v2.c              //PCM module platform driver
- VoIP platform driver − kernel/msm-5.4/techpack/audio/asoc/msm-pcm-voip-v2.c                //PCM module platform driver
- // ALSA codec drivers − /kernel/sound/soc/codecs/wcdXXXX.c, wcdXXXX-tables.c, for example, WCD9330 files are wcd9330.c, wcd9330-tables.c.  
- // I2C/SLIMbus drivers − /kernel/drivers/slimbus/slim-msm-ctrl.c, slimbus.c  
- ASM driver − kernel/msm-5.4/techpack/audio/dsp/q6asm.c
- ADM driver − kernel/msm-5.4/techpack/audio/dsp/q6adm.c 
- AFE driver − kernel/msm-5.4/techpack/audio/dsp/q6afe.c  
- Voice driver − kernel/msm-5.4/techpack/audio/dsp/q6voice.c 
- // Platform-specific machine driver − msmXXXX.c or apqXXXX.c where XXXX is the chipset version; for example, MSM8960 would be msm8960.c.  


***
### ASOC Machine driver
ref: kernel/msm-5.4/techpack/audio/asoc/sa6155.c
```
module_init(sa6155_init);
	platform_driver_register(&sa6155_asoc_machine_driver);

static struct platform_driver sa6155_asoc_machine_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = sa6155_asoc_machine_of_match,
	},
	.probe = msm_asoc_machine_probe,
	.remove = msm_asoc_machine_remove,
};

const struct dev_pm_ops snd_soc_pm_ops = {
	.suspend = snd_soc_suspend,
	.resume = snd_soc_resume,
	.freeze = snd_soc_suspend,
	.thaw = snd_soc_resume,
	.poweroff = snd_soc_poweroff,
	.restore = snd_soc_resume,
};

static const struct of_device_id sa6155_asoc_machine_of_match[]  = {
	{ .compatible = "qcom,sa6155-asoc-snd-adp-star",
	  .data = "adp_star_codec"},
	{ .compatible = "qcom,sa6155-asoc-snd-custom",
	  .data = "custom_codec"},
	{},
};


(kernel/msm-5.4/techpack/audio/asoc/sa6155.c)
msm_asoc_machine_probe
	card = populate_snd_card_dailinks(&pdev->dev);  // dai links信息
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, pdata);
	ret = snd_soc_of_parse_card_name(card, "qcom,model");
	ret = msm_populate_dai_link_component_of_node(card);
	ret = msm_tdm_init(pdev);
	(kernel/msm-5.4/sound/soc/soc-devres.c)
	ret = devm_snd_soc_register_card(&pdev->dev, card);
		(kernel/msm-5.4/sound/soc/soc-core.c)
		ret = snd_soc_register_card(card);
			snd_soc_bind_card(card);
				ret = snd_soc_instantiate_card(card);
					ret = soc_init_dai_link(card, dai_link);
					snd_soc_dapm_init(&card->dapm, card, NULL);
					ret = soc_bind_dai_link(card, dai_link); // 绑定dai link
						rtd = soc_new_pcm_runtime(card, dai_link);
						rtd->cpu_dai = snd_soc_find_dai(dai_link->cpus); //cpus dai匹配
							snd_soc_is_matching_component(dlc, component) // 先匹配of_node,
							// 然后如果dai_name不为空，比较组件驱动名字和dai_link中cpu_dai_name
							strcmp(dai->name, dlc->dai_name)
						snd_soc_rtdcom_add(rtd, rtd->cpu_dai->component);
						rtd->codec_dais[i] = snd_soc_find_dai(codec); //Find CODEC from registered CODECs
						snd_soc_rtdcom_add(rtd, rtd->codec_dais[i]->component);
						snd_soc_is_matching_component(platform, component)
						snd_soc_rtdcom_add(rtd, component);
						soc_add_pcm_runtime(card, rtd); //将rtd->list加入到card->rtd_list里
							list_add_tail(&rtd->list, &card->rtd_list);
							rtd->num = card->num_rtd; // 设备号，该num即为pcm设备号
							card->num_rtd++;  // 声卡的运行时例+1
					ret = soc_bind_aux_dev(card);
					ret = snd_soc_add_dai_link(card, dai_link);
					ret = snd_card_new(card->dev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1, card->owner, 0, &card->snd_card);
						err = snd_ctl_create(card);  //create control core, called from init.c
						err = snd_info_card_create(card);  //create a card proc file, called from init.c
					soc_init_card_debugfs(card);
					soc_resume_init(card);
					ret = snd_soc_dapm_new_controls(&card->dapm, card->dapm_widgets, card->num_dapm_widgets);
					ret = snd_soc_dapm_new_controls(&card->dapm, card->of_dapm_widgets, card->num_of_dapm_widgets);
					ret = card->probe(card);
					ret = soc_probe_link_components(card);
					ret = soc_probe_aux_devices(card);
					ret = soc_init_dai_link(card, dai_link); //Find new DAI links added during probing components and bind them,
					ret = soc_bind_dai_link(card, dai_link); //Components with topology may bring new DAIs and DAI links
					ret = soc_probe_link_dais(card);  //probe all DAI links on this card 
					soc_link_init(card, rtd);
						ret = soc_rtd_init(rtd, dai_link->name);
						soc_dpcm_debugfs_add(rtd);  //add DPCM sysfs entries
						ret = snd_soc_dai_compress_new(cpu_dai, rtd, num); //create compress_device if possible
							dai->driver->compress_new(rtd, num);
						(kernel/msm-5.4/sound/soc/soc-pcm.c)
						ret = soc_new_pcm(rtd, num);  //create the pcm
							ret = snd_pcm_new(rtd->card->snd_card, new_name, num, playback, capture, &pcm);
								(kernel/msm-5.4/sound/core/pcm.c)
								_snd_pcm_new(card, id, device, playback_count, capture_count, false, rpcm); //pcm的两个流创建，并将pcm设备加到card->devices list里
									static struct snd_device_ops ops = {
										.dev_free = snd_pcm_dev_free,
										.dev_register =	snd_pcm_dev_register,
										.dev_disconnect = snd_pcm_dev_disconnect,
									};
									pcm = kzalloc(sizeof(*pcm), GFP_KERNEL);
									err = snd_pcm_new_stream(pcm, SNDRV_PCM_STREAM_PLAYBACK, playback_count);
									err = snd_pcm_new_stream(pcm, SNDRV_PCM_STREAM_CAPTURE, capture_count);
										snd_device_initialize(&pstr->dev, pcm->card);
										dev_set_name(&pstr->dev, "pcmC%iD%i%c", pcm->card->number, pcm->device, stream == SNDRV_PCM_STREAM_PLAYBACK ? 'p' : 'c');
										err = snd_pcm_substream_proc_init(substream);
										snd_pcm_group_init(&substream->self_group);
									(kernel/msm-5.4/sound/core/device.c)
									err = snd_device_new(card, SNDRV_DEV_PCM, pcm, internal ? &internal_ops : &ops);
										dev = kzalloc(sizeof(*dev), GFP_KERNEL);
										list_add(&dev->list, p);
							snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &rtd->ops); //注册playback的 substream->ops
							snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &rtd->ops);  //注册capture的 substream->ops
							ret = snd_soc_pcm_component_new(pcm);
						ret = soc_link_dai_pcm_new(&cpu_dai, 1, rtd);
					snd_soc_dapm_link_dai_widgets(card);
					snd_soc_dapm_connect_dai_link_widgets(card);
					ret = snd_soc_add_card_controls(card, card->controls, card->num_controls);
					ret = snd_soc_dapm_add_routes(&card->dapm, card->dapm_routes, card->num_dapm_routes);
					ret = snd_soc_dapm_add_routes(&card->dapm, card->of_dapm_routes, card->num_of_dapm_routes);
					snd_soc_dapm_new_widgets(card);
					(kernel/msm-5.4/sound/core/init.c)
					ret = snd_card_register(card->snd_card);
						err = device_add(&card->card_dev);
						(kernel/msm-5.4/sound/core/device.c)
						err = snd_device_register_all(card)
							err = __snd_device_register(dev);
								int err = dev->ops->dev_register(dev);
								(此函数最终会调用各个devices的snd_device_ops中的dev_register函数注册各个device)
								(kernel/msm-5.4/sound/core/pcm.c)
								snd_pcm_dev_register() // for pcm device  pcm.c
									err = snd_pcm_add(pcm);
									(kernel/msm-5.4/sound/core/sound.c)
									err = snd_register_device(devtype, pcm->card, pcm->device, &snd_pcm_f_ops[cidx], pcm, &pcm->streams[cidx].dev); //register pcm
								snd_ctl_dev_register() // for control device  control.c
								snd_timer_dev_register() // for timer device  timer.c
					dapm_mark_endpoints_dirty(card);
					snd_soc_dapm_sync(&card->dapm);	
	ret = msm_get_pinctrl(pdev);
	msm_i2s_auxpcm_init(pdev);
	ret = msm_audio_ssr_register(&pdev->dev);



（kernel/msm-5.4/sound/soc/soc-pcm.c）
int soc_new_pcm(struct snd_soc_pcm_runtime *rtd, int num)中调用snd_pcm_set_ops函数设置了substream->ops

	/* ASoC PCM operations */
	if (rtd->dai_link->dynamic) {
		rtd->ops.open		= dpcm_fe_dai_open;
		rtd->ops.hw_params	= dpcm_fe_dai_hw_params;
		rtd->ops.prepare	= dpcm_fe_dai_prepare;
		rtd->ops.trigger	= dpcm_fe_dai_trigger;
		rtd->ops.hw_free	= dpcm_fe_dai_hw_free;
		rtd->ops.close		= dpcm_fe_dai_close;
		rtd->ops.pointer	= soc_pcm_pointer;
		rtd->ops.ioctl		= snd_soc_pcm_component_ioctl;
#ifdef CONFIG_AUDIO_QGKI
		rtd->ops.compat_ioctl   = soc_pcm_compat_ioctl;
		rtd->ops.delay_blk	= soc_pcm_delay_blk;
#endif
	} else {
		rtd->ops.open		= soc_pcm_open;
		rtd->ops.hw_params	= soc_pcm_hw_params;
		rtd->ops.prepare	= soc_pcm_prepare;
		rtd->ops.trigger	= soc_pcm_trigger;
		rtd->ops.hw_free	= soc_pcm_hw_free;
		rtd->ops.close		= soc_pcm_close;
		rtd->ops.pointer	= soc_pcm_pointer;
		rtd->ops.ioctl		= snd_soc_pcm_component_ioctl;
#ifdef CONFIG_AUDIO_QGKI
		rtd->ops.compat_ioctl   = soc_pcm_compat_ioctl;
		rtd->ops.delay_blk	= soc_pcm_delay_blk;
#endif
	}

	if (playback)
		snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &rtd->ops);

	if (capture)
		snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &rtd->ops);
```


***
### ASOC Platform driver
ref: kernel/msm-5.4/sound/soc/soc-core.c  
platform driver应该只是注册一些结构函数指针，没有匹配设备树调用soc_probe，相应的snd_soc_register_card函数应该是在machine driver匹配时调用的  TBD  
```
(kernel/msm-5.4/sound/soc/soc-core.c)
module_init(snd_soc_init);
	snd_soc_init(void)
		snd_soc_debugfs_init();
		snd_soc_util_init();
		platform_driver_register(&soc_driver);

/* ASoC platform driver */
static struct platform_driver soc_driver = {
	.driver		= {
		.name		= "soc-audio",
		.pm		= &snd_soc_pm_ops,
	},
	.probe		= soc_probe,
	.remove		= soc_remove,
};

const struct dev_pm_ops snd_soc_pm_ops = {
	.suspend = snd_soc_suspend,
	.resume = snd_soc_resume,
	.freeze = snd_soc_suspend,
	.thaw = snd_soc_resume,
	.poweroff = snd_soc_poweroff,
	.restore = snd_soc_resume,
};

//static int soc_probe(struct platform_device *pdev)
//	struct snd_soc_card *card = platform_get_drvdata(pdev);
//	snd_soc_register_card(card); //
//		snd_soc_bind_card(card);
//			ret = snd_soc_instantiate_card(card);
//				...

```


***
### PCM open & write call stack
```
声卡
--> 播放流
  --> pcm子流
    --> dpcm前端dai
      --> 后端所有组件打开
        --> 前端所有组件打开 (按照fe dai, codec组件，cpu组件顺序)

// 似乎没有用到这里的snd_open
(kernel/msm-5.4/sound/core/sound.c)
static const struct file_operations snd_fops =
{
	.owner =	THIS_MODULE,
	.open =		snd_open,
	.llseek =	noop_llseek,
};

(kernel/msm-5.4/sound/core/pcm_native.c)
const struct file_operations snd_pcm_f_ops[2] = {
	{
		.owner =		THIS_MODULE,
		.write =		snd_pcm_write,
		.write_iter =		snd_pcm_writev,
		.open =			snd_pcm_playback_open,
		.release =		snd_pcm_release,
		.llseek =		no_llseek,
		.poll =			snd_pcm_poll,
		.unlocked_ioctl =	snd_pcm_ioctl,
		.compat_ioctl = 	snd_pcm_ioctl_compat,
		.mmap =			snd_pcm_mmap,
		.fasync =		snd_pcm_fasync,
		.get_unmapped_area =	snd_pcm_get_unmapped_area,
	},
	{
		.owner =		THIS_MODULE,
		.read =			snd_pcm_read,
		.read_iter =		snd_pcm_readv,
		.open =			snd_pcm_capture_open,
		.release =		snd_pcm_release,
		.llseek =		no_llseek,
		.poll =			snd_pcm_poll,
		.unlocked_ioctl =	snd_pcm_ioctl,
		.compat_ioctl = 	snd_pcm_ioctl_compat,
		.mmap =			snd_pcm_mmap,
		.fasync =		snd_pcm_fasync,
		.get_unmapped_area =	snd_pcm_get_unmapped_area,
	}
};

// tinyalsa open
// PcmTinyAlsaWrapper是我们对tinyalsa的封装
vendor/harman/hardware/audio/audiohal/harman_audio_router/Modules/src/PcmTinyAlsaWrapper.cpp
int PcmTinyAlsaWrapper::openPcmNode()
	external/tinyalsa/pcm.c
	pcm_open(mCardID, mDeviceID, flags, &mConfig);
		pcm->fd = open(fn, O_RDWR|O_NONBLOCK);  //打开底层设备节点
			kernel/msm-5.4/sound/core/pcm_native.c
			static int snd_pcm_playback_open(struct inode *inode, struct file *file)
		ioctl(pcm->fd, SNDRV_PCM_IOCTL_INFO, &info)) //get info
		ioctl(pcm->fd, SNDRV_PCM_IOCTL_HW_PARAMS, &params) //将硬件参数参数写到设备节点中
		pcm->mmap_buffer = mmap(NULL, pcm_frames_to_bytes(pcm, pcm->buffer_size), PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, pcm->fd, 0);  //将pcm->buffer_size映射进内存
		ioctl(pcm->fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sparams) //将软件参数写入到底层设备节点中

// tinyalsa open，接snd_pcm_playback_open
kernel/msm-5.4/sound/core/pcm_native.c
static int snd_pcm_playback_open(struct inode *inode, struct file *file)
    err = snd_pcm_open(file, pcm, SNDRV_PCM_STREAM_PLAYBACK);
        err = snd_card_file_add(pcm->card, file);
        err = snd_pcm_open_file(file, pcm, stream);
            err = snd_pcm_open_substream(pcm, stream, file, &substream);
                err = snd_pcm_attach_substream(pcm, stream, file, &substream); //获取pcm的substream
                err = snd_pcm_hw_constraints_init(substream);  //初始化pcm硬件约束
                (kernel/msm-5.4/sound/soc/soc-pcm.c)
                if ((err = substream->ops->open(substream)) < 0)  //调用substream的回调函数open,注册声卡时已添加
                (call: static int dpcm_fe_dai_open(struct snd_pcm_substream *fe_substream))
                    ret = dpcm_path_get(fe, stream, &list);  //找到所有已经激活的链路（widgets）
                    dpcm_process_paths(fe, stream, &list, 1); //calculate valid and active FE <-> BE dpcms
                    ret = dpcm_fe_dai_startup(fe_substream);
                        dpcm_set_fe_update_state(fe, stream, SND_SOC_DPCM_UPDATE_FE);
                        ret = dpcm_be_dai_startup(fe, fe_substream->stream); // BE组件打开
                            soc_pcm_open() // 同下fe打开
                        ret = soc_pcm_open(fe_substream);  //start the DAI frontend
                            pinctrl_pm_select_default_state(cpu_dai->dev);
                            pinctrl_pm_select_default_state(codec_dai->dev);
                            snd_soc_set_runtime_hwparams(substream, &no_host_hardware);
                            ret = snd_soc_dai_startup(cpu_dai, substream);  //startup the audio subsystem
                            ret = soc_pcm_components_open(substream, &component); //fe dai,codec组件，cpu组件都打开
                                ret = snd_soc_component_module_get_when_open(component);
                                ret = snd_soc_component_open(component, substream);
                                    component->driver->ops->open(substream); //调用各个组件的open函数
                                        msm_pcm_open(struct snd_pcm_substream *substream)
                                            runtime->hw = msm_pcm_hardware_playback;  // if music, msm-pcm-q6-v2.c
                                            runtime->hw = msm_pcm_hardware_capture;   // if music, msm-pcm-q6-v2.c
                                            runtime->hw = msm_pcm_hardware;  // if voice, msm-pcm-voice-v2.c
                                            snd_soc_set_runtime_hwparams(substream, &dummy_pcm_hardware); //msm-pcm-loopback-v2.c
                            ret = snd_soc_dai_startup(codec_dai, substream); //codec dai的startup函数
                            ret = rtd->dai_link->ops->startup(substream);
                            snd_soc_runtime_activate(rtd, substream->stream);
                        dpcm_set_fe_runtime(fe_substream);
                        snd_pcm_limit_hw_rates(runtime);
                        ret = dpcm_apply_symmetry(fe_substream, stream);
                    dpcm_clear_pending_state(fe, stream);
                    dpcm_path_put(&list);
                err = snd_pcm_hw_constraints_complete(substream);  //设置pcm硬件约束
            pcm_file->substream = substream;  //将打开的substream保存在pcm_file中
            file->private_data = pcm_file;   //file获得打开的substream



// tinyalsa write 
// PcmTinyAlsaWrapper是我们对tinyalsa的封装
vendor/harman/hardware/audio/audiohal/harman_audio_router/Modules/src/PcmTinyAlsaWrapper.cpp
PcmTinyAlsaWrapper::writeFrame(AudioPort* port, PAF_AudioFrame* data)
	external/tinyalsa/pcm.c
	pcm_mmap_write(mPcmHandle, static_cast<const void*>(pcmData), mAudioParam.convertFramesToBytes(period_size));
	pcm_write(mPcmHandle, static_cast<const void*>(pcmData), mAudioParam.convertFramesToBytes(period_size));
		ioctl(pcm->fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &x)
		kernel/msm-5.4/sound/core/pcm_native.c
		snd_pcm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
			snd_pcm_common_ioctl(file, pcm_file->substream, cmd, (void __user *)arg);
				snd_pcm_xferi_frames_ioctl(substream, arg);
					copy_from_user(&xferi, _xferi, sizeof(xferi))
					snd_pcm_lib_write(substream, xferi.buf, xferi.frames);
						kernel/msm-5.4/sound/core/pcm_lib.c
						__snd_pcm_lib_xfer(substream, (void __force *)buf, true, frames, false);
							transfer = substream->ops->copy_kernel  //拷贝数据,将数据拷贝到dai FIFO？
							transfer = (pcm_transfer_f)substream->ops->copy_user;
							// 这里到底是不是调用了msm_pcm_copy？ msm_pcm_copy中会调用q6asm_write往dsp写数据
							kernel/msm-5.4/sound/core/pcm_native.c
							snd_pcm_start(substream);
								snd_pcm_action(&snd_pcm_action_start, substream, SNDRV_PCM_STATE_RUNNING);
									snd_pcm_action_group(ops, substream, state, 1);
									snd_pcm_action_single(ops, substream, state);
										res = ops->pre_action(substream, state);
										res = ops->do_action(substream, state);
										snd_pcm_do_start(struct snd_pcm_substream *substream, int state)
											substream->ops->trigger(substream, SNDRV_PCM_TRIGGER_START); //触发数据拷贝
											// 这里到底是不是调用了msm_pcm_trigger？



// 不确定substream->ops->copy_user调用的是不是如下函数  --TBD
// 此函数是应该是发送音频数据
static int msm_pcm_copy(struct snd_pcm_substream *substream, int a, unsigned long hwoff, void __user *buf, unsigned long fbytes)
	ret = msm_pcm_capture_copy(substream, a, hwoff, buf, fbytes); //substream->stream == SNDRV_PCM_STREAM_CAPTURE
	ret = msm_pcm_playback_copy(substream, a, hwoff, buf, fbytes); //substream->stream == SNDRV_PCM_STREAM_PLAYBACK
		copy_from_user(bufptr, buf, xfer)  //将数据拷贝到内核空间audio_client对应的buffer
		ret = q6asm_write(prtd->audio_client, xfer, 0, 0, NO_TIMESTAMP);
			q6asm_update_token(&write.hdr.token,
				   0, /* Session ID is NA */
				   0, /* Stream ID is NA */
				   port->dsp_buf,
				   0, /* Direction flag is NA */
				   NO_WAIT_CMD);
			rc = apr_send_pkt(ac->apr, (uint32_t *) &write);
				rc = apr_tal_write(clnt->handle, buf, (struct apr_pkt_priv *)&svc->pkt_owner, hdr->pkt_size); //native
				ret = habmm_socket_send(hab_handle_tx, (void *)&apr_tx_buf, apr_send_len, 0);  //vm,qnx hypervisor


// 不确定substream->ops->trigger调用的是不是如下函数  --TBD
// 此函数应该是发送操作码命令
kernel/msm-5.4/techpack/audio/asoc/msm-pcm-q6-v2.c
kernel/msm-5.4/techpack/audio/asoc/msm-pcm-voice-v2.c
kernel/msm-5.4/techpack/audio/asoc/msm-pcm-voip-v2.c
kernel/msm-5.4/techpack/audio/asoc/msm-pcm-loopback-v2.c
kernel/msm-5.4/techpack/audio/asoc/msm-pcm-q6-noirq.c
static int msm_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
	kernel/msm-5.4/techpack/audio/dsp/q6asm.c    //项目用到的目录
	kernel/msm-5.4/sound/soc/qcom/qdsp6/q6asm.c  //这个应该是原始的目录
	ret = q6asm_run_nowait(prtd->audio_client, 0, 0, 0);
		kernel/msm-5.4/techpack/audio/dsp/q6asm.c
		__q6asm_cmd_nowait(ac, cmd, ac->stream_id);
			q6asm_update_token(&hdr.token, ac->session, stream_id, 0, 0, NO_WAIT_CMD);
			kernel/msm-5.4/techpack/audio/ipc/apr_vm.c
			rc = apr_send_pkt(ac->apr, (uint32_t *) &hdr);
				rc = apr_tal_write(clnt->handle, buf, (struct apr_pkt_priv *)&svc->pkt_owner, hdr->pkt_size); //native
				ret = habmm_socket_send(hab_handle_tx, (void *)&apr_tx_buf, apr_send_len, 0);  //vm,qnx hypervisor



// 不确定substream->ops->trigger调用的是不是如下函数  --TBD
substream->ops->trigger(substream, SNDRV_PCM_TRIGGER_START);
	dpcm_fe_dai_trigger(struct snd_pcm_substream *substream, int cmd)
		dpcm_fe_dai_do_trigger(substream, cmd);
			dpcm_dai_trigger_fe_be(substream, cmd, true);
				ret = soc_pcm_trigger(substream, cmd);
					ret = snd_soc_dai_trigger(codec_dai, substream, cmd);
					ret = snd_soc_component_trigger(component, substream, cmd);
					ret = snd_soc_dai_trigger(cpu_dai, substream, cmd);
				ret = dpcm_be_dai_trigger(fe, substream->stream, cmd);
										
					
```



// need to analyze  
kernel/msm-5.4/techpack/audio/asoc/msm-pcm-q6-v2.c
```
static const struct snd_pcm_ops msm_pcm_ops = {
	.open           = msm_pcm_open,
	.copy_user	= msm_pcm_copy,
	.hw_params	= msm_pcm_hw_params,
	.close          = msm_pcm_close,
	.ioctl          = msm_pcm_ioctl,
#if IS_ENABLED(CONFIG_AUDIO_QGKI)
	.compat_ioctl   = msm_pcm_compat_ioctl,
#endif /* CONFIG_AUDIO_QGKI */
	.prepare        = msm_pcm_prepare,
	.trigger        = msm_pcm_trigger,
	.pointer        = msm_pcm_pointer,
	.mmap		= msm_pcm_mmap,
};
```





------------------------------------------------------------
Generated with [Mybase Desktop 8.2 Beta-5](http://www.wjjsoft.com/mybase.html?ref=markdown_export)
