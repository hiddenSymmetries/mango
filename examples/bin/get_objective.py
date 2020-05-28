#!/usr/bin/env python                                                                                       
#import numpy as np

def get_initial_conditions():
    x1 = -1.2
    x2 = 1.0
    x0 = [x1,x2]
    return x0

def check_input(x0):
    print(x0)
    return x0

def get_objective(state_vec, return_sigma_and_target):

    ######### Set desired objective function
    obj_func = "rosenbrock";
    
    if obj_func == "rosenbrock":
        out_list = rosenbrock(obj_func, state_vec, return_sigma_and_target)
    else:
        print("This function is not currently available. Exiting...")
        exit()
        
    return out_list

def rosenbrock(obj_func, state_vec, return_sigma_and_target):
    # f = [10*(x1-x2^2)]^2 + [1-x1]^2
    # r1 = x1-x2^2 --> sigma = 0.1 --> target = 0.0
    r1 = state_vec[0] - state_vec[1]*state_vec[1]
    # r2 = x1 --> sigma = 1.0 --> target = 1.0
    r2 = state_vec[0]
    resid = [r1, r2]
    if (return_sigma_and_target == 1):
        sigma = [0.1, 1.0]
        target = [0.0, 1.0]
        return [resid, sigma, target, obj_func]
    else:
        return resid
