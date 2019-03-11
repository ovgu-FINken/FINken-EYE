// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// JeVois Smart Embedded Machine Vision Toolkit - Copyright (C) 2016 by Laurent Itti, the University of Southern
// California (USC), and iLab at USC. See http://iLab.usc.edu and http://jevois.org for information about this project.
//
// This file is part of the JeVois Smart Embedded Machine Vision Toolkit.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software
// Foundation, version 2.  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.  You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
// Contact information: Laurent Itti - 3641 Watt Way, HNB-07A - Los Angeles, CA 90089-2520 - USA.
// Tel: +1 213 740 3527 - itti@pollux.usc.edu - http://iLab.usc.edu - http://jevois.org
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \file */

#include <jevois/Core/Module.H>
#include <jevois/Image/RawImageOps.H>
#include <jevois/Debug/Timer.H>

#include <svo/config.h>
#include <svo/frame_handler_mono.h>
#include <svo/map.h>
#include <svo/frame.h>
#include <vikit/math_utils.h>
#include <vikit/vision.h>
#include <vikit/abstract_camera.h>
#include <vikit/atan_camera.h>
#include <vikit/pinhole_camera.h>
//#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sophus/se3.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <atomic>

// icon by Catalin Fertu in cinema at flaticon

/*!
    @author Marcel Plumbohm

    @videomapping YUYV 640 480 28.5 YUYV 640 480 28.5 FINken_Nano svo_no_ros
    @license GPL v3
    @distribution Unrestricted
    @restrictions None */  
     
class svo_no_ros : public jevois::Module
{
	protected:
		vk::AbstractCamera* cam_;
		svo::FrameHandlerMono* vo_;
		int img_id;
		std::ofstream file;
		std::future<void> readyUpSvo;
		std::future<void> svoProcess;
		std::atomic<bool> svo_ready;
		std::atomic<int> frame_id;
		std::atomic<int> num_features;
		std::atomic<int> proc_time;
		//cv::FileStorage imagefile;
		void writeToFile(std::string const alg_result) {
		  file << alg_result + "\n";
		  
		  return;
		}

  public:
    svo_no_ros(std::string const & instance) : jevois::Module(instance), svo_ready(false), img_id(0), frame_id(0), num_features(0), proc_time(0) {
    	// TODO Camera configuration
			//cam_ = new vk::PinholeCamera(640, 480, 315.5, 315.5, 376.0, 240.0);
			cam_ = new vk::ATANCamera(640.0, 480.0, 1.0, 1.0, 1.0, 1.0, 1.0);
			vo_ = new svo::FrameHandlerMono(cam_);
			file.open("svo_log.txt");	
			//imagefile = cv::FileStorage("some_name.png", cv::FileStorage::WRITE);
    }

		void postInit() override {
		  readyUpSvo = std::async(std::launch::async, [&]() {
		    vo_->start();
		    svo_ready.store(true);
		  });
		}

    ~svo_no_ros() {
      delete vo_;
  		delete cam_;
		  file.close();
		}
		
		void postUninit() override {
		  if(readyUpSvo.valid()) try { readyUpSvo.get(); } catch (...) { }
		  try {svoProcess.get(); } catch (...) { }
		  if(vo_) { free(vo_); vo_ = nullptr; }
		  if(cam_) { free(cam_); cam_ = nullptr; }
		}

// ###################################################################################################

    //! Processing function, no video output
    virtual void process(jevois::InputFrame && inframe) override
    {
      // Wait for next available camera image:
      jevois::RawImage const inimg = inframe.get();
      unsigned int const w = inimg.width, h = inimg.height;

      // Convert input image to RGB for predictions:
      cv::Mat cvimg = jevois::rawimage::convertToCvGray(inimg);
      
      //imagefile << 
      
      // Start SVO processing
      if (svo_ready.load()) {
        vo_->addImage(cvimg, 0.01*img_id);
      }

      // Write results to log
      if(vo_->lastFrame() != NULL)
    {
      writeToFile("Frame Id: " + std::to_string(vo_->lastFrame()->id_) + "  " + "#Features: " + std::to_string(vo_->lastNumObservations()) + "  " + "Processing Time: " + std::to_string(vo_->lastProcessingTime()*1000) + "  "); //+ std::to_string(vo_->lastFrame()->T_f_w_));   
    }      
      img_id++;
      
      // Let camera know we are done processing the input image:
      inframe.done();
    }
    
//###################################################################################################
    
    //! Processing function
    virtual void process(jevois::InputFrame && inframe, jevois::OutputFrame && outframe) override
    {
			static jevois::Timer timer("processing", 50, LOG_DEBUG);
			
      // Wait for next available camera image:
      jevois::RawImage const inimg = inframe.get();
      inimg.require("input", inimg.width, inimg.height, V4L2_PIX_FMT_YUYV);
      
	    timer.start();

      // Start thread to paste input in out frame
      jevois::RawImage outimg;
      auto paste_fut = std::async(std::launch::async, [&]() {
        outimg = outframe.get();
        outimg.require("output", inimg.width, inimg.height, inimg.fmt);
        
        jevois::rawimage::paste(inimg, outimg, 0, 0);
        
        if(frame_id.load() != 0 && frame_id.load() > 2) {
          jevois::rawimage::writeText(outimg, "Frame Id: " + to_string(frame_id), 50, 300, jevois::yuyv::White, jevois::rawimage::Font15x28);
      	  jevois::rawimage::writeText(outimg, "#Features: " + to_string(num_features), 50, 340, jevois::yuyv::White, jevois::rawimage::Font15x28);
      	  jevois::rawimage::writeText(outimg, "Svo Processing time: " + to_string(proc_time), 50, 380, jevois::yuyv::White, jevois::rawimage::Font15x28);
      	  } else {
      	  jevois::rawimage::writeText(outimg, "Waiting for SVO data...", 50, 300, jevois::yuyv::White, jevois::rawimage::Font15x28);
      	}
      });
      if (svoProcess.valid()) {
        if (svoProcess.wait_for(std::chrono::milliseconds(5)) == std::future_status::ready)
        {
          svoProcess.get();
        }
        
        // Wait for paste to finish up:
        paste_fut.get();

        // Let camera know we are done processing the input image:
        inframe.done();
      }
      else {
        if (svo_ready.load()) {
          // Convert raw image into RGB OpenCV image and add frame to the algorithm
          cv::Mat cvimg = jevois::rawimage::convertToCvGray(inimg);
          
          svoProcess = std::async(std::launch::async, [&]() {
            vo_->addImage(cvimg, 0.01*img_id);
            
            if(vo_->lastFrame() != NULL) {
            frame_id.store(vo_->lastFrame()->id_);
            num_features.store(vo_->lastNumObservations());
            proc_time.store(int(vo_->lastProcessingTime()*1000));
            //std::to_string(vo_->lastFrame()->T_f_w_);
            }
          });     
        }
        
        // Wait for paste to finish up:
        paste_fut.get();

        // Let camera know we are done processing the input image:
        inframe.done();
      }
      //	process.get();     
        //writeToFile(frame_id + "  " + num_features + "  " + proc_time + "  " + jevois_proc_time);     	

      // Show processing fps:
      std::string const & fpscpu = timer.stop();
      jevois::rawimage::writeText(outimg, fpscpu, 3, 420, jevois::yuyv::White);

      // Send the output image with our processing results to the host over USB:
      outframe.send(); // NOTE: optional here, outframe destructor would call it anyway
      
      img_id++;
    }
};
// Allow the module to be loaded as a shared object (.so) file:
JEVOIS_REGISTER_MODULE(svo_no_ros);
