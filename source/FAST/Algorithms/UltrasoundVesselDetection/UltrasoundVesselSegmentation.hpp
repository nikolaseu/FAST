#ifndef ULTRASOUND_VESSEL_SEGMENTATION_HPP
#define ULTRASOUND_VESSEL_SEGMENTATION_HPP

#include "FAST/ProcessObject.hpp"
#include <deque>

namespace fast {

class VesselCrossSection;
class KalmanFilter;
class UltrasoundVesselDetection;
class Image;

class FAST_EXPORT  TrackedVessel : public Object {
	FAST_OBJECT(TrackedVessel)
public:
	std::shared_ptr<KalmanFilter> filter;
	int framesSinceRefshed;
};

class FAST_EXPORT  UltrasoundVesselSegmentation : public ProcessObject {
	FAST_OBJECT(UltrasoundVesselSegmentation)
	public:

	private:
		UltrasoundVesselSegmentation();
		void execute();

		int mFramesToKeep;
		/*
		 * This stores all the vessel cross section detected for N frames back
		 */
		std::deque<std::vector<std::shared_ptr<VesselCrossSection> > > mDetectedVessels;

		/*
		 * This keeps a Kalman filter for all the tracked vessels
		 */
		std::vector<std::shared_ptr<TrackedVessel> > mKalmanFilters;

		/*
		 * This keeps the ultrasound vessel detection object
		 */
		std::shared_ptr<UltrasoundVesselDetection> mDetector;

		void checkForNewVessels(float spacing);
		void trackAllVessels(std::shared_ptr<Image> image);
		void createSegmentation(std::shared_ptr<Image> image);
};

} // namespace fast


#endif
