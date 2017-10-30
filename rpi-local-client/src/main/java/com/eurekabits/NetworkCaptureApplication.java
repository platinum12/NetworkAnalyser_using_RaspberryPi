package com.eurekabits;

import java.util.Arrays;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ApplicationContext;

@SpringBootApplication
public class NetworkCaptureApplication {

    public static void main(String[] args) {
        ApplicationContext ctx = SpringApplication.run(NetworkCaptureApplication.class, args);
        
        System.out.println("Starting **Network Packet Capture** application ......");

//        String[] beanNames = ctx.getBeanDefinitionNames();
//        Arrays.sort(beanNames);
//        for (String beanName : beanNames) {
//            System.out.println(beanName);
//        }
    }
}
