/*
 * This is part of the Bayesian Object Tracking (bot),
 * (https://github.com/bayesian-object-tracking)
 *
 * Copyright (c) 2015 Max Planck Society,
 * 				 Autonomous Motion Department,
 * 			     Institute for Intelligent Systems
 *
 * This Source Code Form is subject to the terms of the GNU General Public
 * License License (GNU GPL). A copy of the license can be found in the LICENSE
 * file distributed with this source code.
 */

/**
 * \file rbc_particle_filter_tracker_builder.hpp
 * \date November 2015
 * \author Jan Issac (jan.issac@gmail.com)
 */

#pragma once

#include <exception>

#include <dbot/tracker/rbc_particle_filter_object_tracker.hpp>
#include <dbot/tracker/builder/brownian_motion_model_builder.hpp>
#include <dbot/tracker/builder/object_transition_model_builder.hpp>
#include <dbot/tracker/builder/rb_observation_model_cpu_builder.hpp>

namespace dbot
{
/**
 * \brief The NoGpuSupportException class
 */
class NoGpuSupportException : public std::exception
{
    const char* what() const noexcept
    {
        return "Tracker has not been compiled with GPU support "
               "(DBOT_BUILD_GPU=OFF).";
    }
};

/**
 * \brief Represents an Rbc Particle filter based tracker builder
 */
class RbcParticleFilterTrackerBuilder
{
public:
    typedef osr::FreeFloatingRigidBodiesState<> State;
    typedef Eigen::VectorXd Noise;
    typedef Eigen::VectorXd Input;

    typedef fl::StateTransitionFunction<State, Noise, Input> StateTransition;
    typedef RbObservationModel<State> ObservationModel;
    typedef typename ObservationModel::Observation Obsrv;

    typedef RBCoordinateParticleFilter<StateTransition, ObservationModel>
        Filter;

    struct Parameters
    {
        struct TrackerParmeters
        {
            int evaluation_count;
            int max_sample_count;
            double update_rate;
            double max_kl_divergence;
        };

        bool use_gpu;

        TrackerParmeters cpu;
        TrackerParmeters gpu;
        TrackerParmeters tracker;

        ObjectResourceIdentifier ori;
        RbObservationModelBuilder<State>::Parameters observation;
        ObjectTransitionModelBuilder<State, Input>::Parameters
            object_transition;
        BrownianMotionModelBuilder<State, Input>::Parameters
            brownian_transition;
    };

public:
    /**
     * \brief Creates a RbcParticleFilterTrackerBuilder
     * \param param			Builder and sub-builder parameters
     * \param camera_data	Tracker camera data object
     */
    RbcParticleFilterTrackerBuilder(const Parameters& param,
                                    const CameraData& camera_data);

    /**
     * \brief Builds the Rbc PF tracker
     */
    std::shared_ptr<RbcParticleFilterObjectTracker> build();

private:
    /**
     * \brief Creates an instance of the Rbc particle filter
     */
    std::shared_ptr<Filter> create_filter(const ObjectModel& object_model,
                                          double max_kl_divergence);

    //    /**
    //     * \brief Creates a Brownian motion state transition function used in
    //     the
    //     *        filter
    //     */
    //    std::shared_ptr<StateTransition>
    //    create_brownian_state_transition_model(
    //        const BrownianMotionModelBuilder<State, Input>::Parameters& param)
    //        const;

    /**
     * \brief Creates a Linear object transition function used in the
     *        filter
     */
    std::shared_ptr<StateTransition> create_object_transition_model(
        const ObjectTransitionModelBuilder<State, Input>::Parameters& param)
        const;

    /**
     * \brief Creates the Rbc particle filter observation model. This can either
     *        be CPU or GPU based
     *
     * \throws NoGpuSupportException if compile with DBOT_BUILD_GPU=OFF and
     *         attempting to build a tracker with GPU support
     */
    std::shared_ptr<ObservationModel> create_obsrv_model(
        bool use_gpu,
        const ObjectModel& object_model,
        const CameraData& camera_data,
        const RbObservationModelBuilder<State>::Parameters& param) const;

    /**
     * \brief Loads and creates an object model represented by the specified
     *        resource identifier
     */
    ObjectModel create_object_model(const ObjectResourceIdentifier& ori) const;

    /**
     * \brief Creates a sampling block definition used by the coordinate
     *        particle filter
     *
     * \param blocks		Number of objects or object parts
     * \param block_size	State dimension of each part
     */
    std::vector<std::vector<size_t>> create_sampling_blocks(
        int blocks,
        int block_size) const;

private:
    Parameters param_;
    dbot::CameraData camera_data_;
};
}
