/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
    
    num_particles = 30; // 30 seems to work fine
    is_initialized = true;
    weights = vector<double>(num_particles, 1.0);
    
    // create nGaussian distributions for each parameter
    normal_distribution<double> dist_x(x, std[0]);
    normal_distribution<double> dist_y(y, std[1]);
    normal_distribution<double> dist_theta(theta, std[2]);
    
    // create random particles
    default_random_engine gen;
    for (int i=0; i<num_particles; i++){
        Particle newParticle;
        newParticle.id = i;
        newParticle.weight = 1.0;
        
        newParticle.x = dist_x(gen);
        newParticle.y = dist_y(gen);
        newParticle.theta = dist_theta(gen);
        particles.push_back(newParticle);
    }
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
    default_random_engine gen;
    
    for (Particle& particle : particles){
        // add some noise
        double oldX = normal_distribution<double>(particle.x, std_pos[0])(gen);
        double oldY = normal_distribution<double>(particle.y, std_pos[1])(gen);
        double oldTheta = normal_distribution<double>(particle.theta, std_pos[2])(gen);
        
        // update state
        if (abs(yaw_rate) > 0.00001){
            particle.theta = oldTheta + yaw_rate * delta_t;
            particle.x = oldX + velocity / yaw_rate * (sin(particle.theta) - sin(oldTheta));
            particle.y = oldY + velocity / yaw_rate * (cos(oldTheta) - cos(particle.theta));
        }
        else {
            particle.x = oldX + velocity * delta_t * cos(oldTheta);
            particle.y = oldY + velocity * delta_t * sin(oldTheta);
        }
    }
    
    
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
    // TODO: Find the predicted measurement that is closest to each observed measurement and assign the
    //   observed measurement to this particular landmark.
    // NOTE: this method will NOT be called by the grading code. But you will probably find it useful to
    //   implement this method and use it as a helper during the updateWeights phase.
    
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[],
                                   const std::vector<LandmarkObs> &observations, const Map &map_landmarks) {
    // TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
    //   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
    
    for (Particle& particle : particles){
        vector<int> associations;
        vector<double> sense_x;
        vector<double> sense_y;
        double weight = 1;
        
        for (LandmarkObs obs : observations){
            // transform to map coordinates
            double mapX = particle.x + cos(particle.theta) * obs.x - sin(particle.theta) * obs.y;
            double mapY = particle.y + sin(particle.theta) * obs.x + cos(particle.theta) * obs.y;
            
            //find closest landmark
            double closestDistance = 999999;
            Map::single_landmark_s closestLandmark;
            for (Map::single_landmark_s pred : map_landmarks.landmark_list){
                double distance = dist(mapX, mapY, pred.x_f, pred.y_f);
                if (distance < closestDistance){
                    closestLandmark = pred;
                    closestDistance = distance;
                }
            }
            associations.push_back(closestLandmark.id_i);
            sense_x.push_back(mapX);
            sense_y.push_back(mapY);
            
            weight *= exp(-(mapX-closestLandmark.x_f)*(mapX-closestLandmark.x_f)/(2*std_landmark[0]*std_landmark[0]) -
                          (mapY-closestLandmark.y_f)*(mapY-closestLandmark.y_f)/(2*std_landmark[1]*std_landmark[1]));
        }
        SetAssociations(particle, associations, sense_x, sense_y);
        particle.weight = weight;
    }
}

void ParticleFilter::resample() {
    // generate list of weights
    vector<double> weights;
    for (int i=0; i<particles.size(); i++){
        weights.push_back(particles[i].weight);
    }
    
    // get a distribution based on those weights
    discrete_distribution<int> weightDist (weights.begin(), weights.end());
    default_random_engine generator;
    
    // sample the new distribution
    vector<Particle> newParticles;
    for (int i=0; i<num_particles; i++){
        int index = weightDist(generator);
        newParticles.push_back(particles[index]);
    }
    
    particles = newParticles;
}

Particle ParticleFilter::SetAssociations(Particle& particle, const std::vector<int>& associations,
                                         const std::vector<double>& sense_x, const std::vector<double>& sense_y)
{
    //particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates
    
    particle.associations= associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;
    return particle;
}

string ParticleFilter::getAssociations(Particle best)
{
    vector<int> v = best.associations;
    stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
    vector<double> v = best.sense_x;
    stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
    vector<double> v = best.sense_y;
    stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
