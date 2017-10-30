package com.eurekabits.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.quartz.MethodInvokingJobDetailFactoryBean;
import org.springframework.scheduling.quartz.SchedulerFactoryBean;
import org.springframework.scheduling.quartz.SimpleTriggerFactoryBean;

/**
 * Quartz related configuration to scheduling various Jobs.
 */
@Configuration
@ComponentScan("com.eurekabits")
public class QuartzConfiguration {

    /**
     * MethodInvokingJobDetailFactoryBean exposes org.quartz.JobDetail, which we
     * will need while configuring TriggerFactory.
     * Here we configure the Bean name of the Job and its execution method name.
     *
     * @return 
     */
    @Bean
    public MethodInvokingJobDetailFactoryBean methodInvokingJobDetailFactoryBean() {
        MethodInvokingJobDetailFactoryBean obj = new MethodInvokingJobDetailFactoryBean();
        obj.setTargetBeanName("configSyncJob");
        obj.setTargetMethod("start");
        return obj;
    }

    /**
     * SimpleTriggerFactoryBean implements Quartz org.quartz.SimpleTrigger interface.
     * In this bean we configure JobDetails and its execution policy (startDelay, interval) etc.
     * 
     * @return 
     */
    @Bean
    public SimpleTriggerFactoryBean simpleTriggerFactoryBean() {

        SimpleTriggerFactoryBean stFactory = new SimpleTriggerFactoryBean();
        stFactory.setJobDetail(methodInvokingJobDetailFactoryBean().getObject());
        stFactory.setStartDelay(3000); // 3 secs.
        stFactory.setRepeatInterval(15000); // 15sec x 1000 (ms) = 15,000 ms.
        //stFactory.setRepeatInterval(300000); // 300secs(5min) x 1000 (ms) = 300,000 Ms.
        // Since default behaviour is to repeat indefinitely, don't set here.
        //stFactory.setRepeatCount(3); 
        return stFactory;
    }
    
    /**
     * We register all our triggers in this Bean.
     *
     * @return 
     */
    @Bean
    public SchedulerFactoryBean schedulerFactoryBean() {
        SchedulerFactoryBean scheduler = new SchedulerFactoryBean();
        scheduler.setTriggers(simpleTriggerFactoryBean().getObject());
        return scheduler;
    }
    
}
