# Modified from single cluster input for testing, original codes at https://gitlab.cern.ch/hgcal-tpg/Stage2/-/tree/cp_truth_v16/ClusterProperties-Python/Emulator?ref_type=heads


import pandas as pd
import math
pd.set_option('display.max_columns', None)

import MyConstants

import multiprocessing as mp

import numpy as np
import tqdm as tqdm

import argparse

# Need to define my rounding function as Python makes weird choices for frac = 0.5
def MyRound(FloatNum):
    (Fraction, Quotient) = math.modf(FloatNum)
    Result = (Quotient+1) if ( (Fraction) >= 0.5 ) else Quotient
    return Result
MyRoundVectorized = np.vectorize(MyRound)

# Sigma_Energy Calculator
def Sigma_Energy(N_TC_W,Sum_W2,Sum_W):
    if (N_TC_W<=0) :
        return 0
    N_TC_W = MyConstants.DoubleFloatToSingleFloat( N_TC_W )
    Sum_W2 = MyConstants.DoubleFloatToSingleFloat( Sum_W2 )
    Sum_W = MyConstants.DoubleFloatToSingleFloat( Sum_W )
    N_temp = MyConstants.DoubleFloatToSingleFloat( N_TC_W*Sum_W2 ) - MyConstants.DoubleFloatToSingleFloat( Sum_W**2 )
    N = MyConstants.DoubleFloatToSingleFloat( N_temp )
    D = MyConstants.DoubleFloatToSingleFloat( N_TC_W**2 )
    var_E = MyConstants.DoubleFloatToSingleFloat( N/D )
    if var_E < 0.0:
        sigma_E = MyConstants.DoubleFloatToSingleFloat( 0.0 )
        print('--> Case of negative variance detected: ', var_E)
        print('--> Replacing with var_E = 0.0')
    else:
        sigma_E = MyConstants.DoubleFloatToSingleFloat( math.sqrt(var_E) )
    return sigma_E
Sigma_Energy_vectorized = np.vectorize(Sigma_Energy)

# Mean_coordinate Calculator
def Mean_coordinate(Sum_Wc,Sum_W):
    if (Sum_W<=0) :
        return 0
    Sum_Wc = MyConstants.DoubleFloatToSingleFloat( Sum_Wc )
    Sum_W = MyConstants.DoubleFloatToSingleFloat( Sum_W )
    mean_c = MyConstants.DoubleFloatToSingleFloat( Sum_Wc/Sum_W )
    return mean_c
Mean_coordinate_vectorized = np.vectorize(Mean_coordinate)

# Sigma_coordinate Calculator
def Sigma_Coordinate(Sum_W,Sum_Wc2,Sum_Wc ):
    if (Sum_W<=0) :
        return 0
    Sum_W = MyConstants.DoubleFloatToSingleFloat( Sum_W )
    Sum_Wc2 = MyConstants.DoubleFloatToSingleFloat( Sum_Wc2 )
    Sum_Wc = MyConstants.DoubleFloatToSingleFloat( Sum_Wc )
    N_temp = MyConstants.DoubleFloatToSingleFloat( Sum_W*Sum_Wc2 ) - MyConstants.DoubleFloatToSingleFloat( Sum_Wc**2 )
    N = MyConstants.DoubleFloatToSingleFloat( N_temp )
    D = MyConstants.DoubleFloatToSingleFloat( Sum_W**2 )
    var_c = MyConstants.DoubleFloatToSingleFloat( N/D )
    if var_c < 0.0:
        sigma_c = MyConstants.DoubleFloatToSingleFloat( 0.0 )
        print('--> Case of negative variance detected: ', var_c)
        print('--> Replacing with var_c = 0.0')
    else:
        sigma_c = MyConstants.DoubleFloatToSingleFloat( math.sqrt(var_c) )
    return sigma_c
Sigma_Coordinate_vectorized = np.vectorize(Sigma_Coordinate)

# Energy_ratio Calculator
def Energy_ratio(E_N, E_D, c_scaler):
    if E_D == 0 :
        return 0
    else:
        # Care about single float precision
        E_N = MyConstants.DoubleFloatToSingleFloat( E_N ) 
        E_D = MyConstants.DoubleFloatToSingleFloat( E_D )
        Energy_r = MyConstants.DoubleFloatToSingleFloat( E_N/E_D )
        Energy_r_fp = MyConstants.SingleFloatToFixedPoint( Energy_r, 20, 12)
        Energy_r_scaled = Energy_r_fp * c_scaler 
        Energy_r_int = MyRound( Energy_r_scaled )
        if Energy_r_int == c_scaler :
            Energy_r_sat = c_scaler - 1
        else:
            Energy_r_sat = Energy_r_int
        return Energy_r_sat
Energy_ratio_vectorized = np.vectorize(Energy_ratio)

def ShowerLengthProperties(layerBits):
    counter = 0
    firstLayer = 0
    firstLayerFound = 0
    lastLayer = 0
    lis = []
    empty = []
    for lb in range(len(layerBits)):
        if layerBits[lb] == 1:
            if firstLayerFound == 0:
                firstLayer = lb + 1
                firstLayerFound = 1
            lastLayer = lb + 1
            counter += 1
        else:
            lis.append(counter)
            counter = 0
    showerLen = lastLayer - firstLayer + 1
    if not empty == lis:
        coreShowerLen = max(lis)
    else:  # it has activity in all layers
        coreShowerLen = counter
    return ( firstLayer, lastLayer, showerLen, coreShowerLen )
ShowerLengthPropertiesVectorized = np.vectorize(ShowerLengthProperties)


########################################################################################################################

# Form a LUT dictionary for mu(roz) --> mu(eta) transform
df_mu_eta_LUT = pd.read_csv('../PerformanceStudies/roz_to_eta_transforms/mean_eta_LUT.csv')
dict_mu_eta_LUT = pd.Series( df_mu_eta_LUT.asinh_zor.values, index = df_mu_eta_LUT.mean_roz ).to_dict() 
print("BEGIN mu_eta_LUT")
print(df_mu_eta_LUT)
print(dict_mu_eta_LUT)
print("END mu_eta_LUT")

# Form a LUT dictionary for (mu(roz), sig(roz) ) --> sig(eta) transform
df_sig_eta_LUT = pd.read_csv('../PerformanceStudies/roz_to_eta_transforms/sigma_eta_LUT.csv')
dict_sig_eta_LUT = pd.Series( df_sig_eta_LUT.sigma_eta.values, index = df_sig_eta_LUT.mu_sig_addr ).to_dict()
print("BEGIN sigma_eta_LUT")
print(df_sig_eta_LUT)
print(dict_sig_eta_LUT)
print("END sigma_eta_LUT")

# Something to find min/max of the calculated mean(roz) min/max after dividing integer sums:
mean_roz_data_min = 1000000000
mean_roz_data_max = -1
sigma_roz_data_min = 1000000000 
sigma_roz_data_max = -1

def process_dataframe(df_CS):

    df_CP = pd.DataFrame(\
        columns = [ 'file_ID','event_ID', 'cluster_ID', \
                    # First 32b word
                    'ET', \
                    'e_or_gamma_ET', \
                    'GCT_e_or_gamma_Select_0', \
                    'GCT_e_or_gamma_Select_1', \
                    'GCT_e_or_gamma_Select_2', \
                    'GCT_e_or_gamma_Select_3', \
                    # Second 32b word
                    'Fraction_in_CE_E', \
                    'Fraction_in_core_CE_E', \
                    'Fraction_in_front_CE_H', \
                    'First_Layer', \
                    # Third 32b word
                    'ET_Weighted_Eta', \
                    'ET_Weighted_Phi', \
                    'ET_Weighted_Z', \
                    # Fourth 32b word
                    'Number_of_TCs', \
                    'Saturated_Trigger_Cell', \
                    'Quality_of_Fraction_in_CE_E', \
                    'Quality_of_Fraction_in_core_CE_E', \
                    'Quality_of_Fraction_in_front_CE_H', \
                    'Quality_of_Sigmas_and_Means', \
                    'Saturated_Phi', \
                    'Nominal_Phi', \
                    # Fifth 32b word
                    'Sigma_E', \
                    'Last_Layer', \
                    'Shower_Length', \
                    # Sixth 32b word
                    'Sigma_ZZ', \
                    'Sigma_PhiPhi', \
                    'Core_Shower_Length', \
                    # Seventh 32b word
                    'Sigma_EtaEta', \
                    'Sigma_RoZRoZ', \
                    # Eight 32b word
                    'LayerBits',\
                    'label'],\
        index = df_CS.index
        )

    df_CP['file_ID'] = df_CS['file_ID']
    df_CP['event_ID'] = df_CS['event_ID']
    df_CP['cluster_ID'] = df_CS['cluster_ID']
    df_CP['label'] = df_CS['label']
    df_CP['LayerBits'] = df_CS['LayerBits']

    # Energies 
    df_CP['ET'] = MyRoundVectorized( df_CS['E'] * MyConstants.c_ET_scaler ).astype(int) # Rounding to L1T LSB
    df_CP['e_or_gamma_ET'] = MyRoundVectorized( df_CS['E_EM'] * MyConstants.c_ET_scaler ).astype(int)  # Rounding to L1T LSB
        
    # Energy Fractions
    df_CP['Fraction_in_CE_E'] = Energy_ratio_vectorized(df_CS['E_EM'], df_CS['E'], MyConstants.c_frac_scaler)
    df_CP['Fraction_in_core_CE_E'] = Energy_ratio_vectorized( df_CS['E_EM_core'], df_CS['E_EM'] , MyConstants.c_frac_scaler )
    df_CP['Fraction_in_front_CE_H'] = Energy_ratio_vectorized( df_CS['E_H_early'], df_CS['E'] , MyConstants.c_frac_scaler )  

    # GCT_e_or_gamma_Select bits (0,1,2,3)
    df_CP['GCT_e_or_gamma_Select_0'] = df_CP.apply(lambda row: 1 if row['Fraction_in_CE_E'] > MyConstants.c_GCT_0 else 0, axis=1)
    df_CP['GCT_e_or_gamma_Select_1'] = df_CP.apply(lambda row: 1 if row['Fraction_in_core_CE_E'] > MyConstants.c_GCT_1 else 0, axis=1)
    df_CP['GCT_e_or_gamma_Select_2'] = df_CP.apply(lambda row: 1 if row['Fraction_in_front_CE_H'] > MyConstants.c_GCT_2 else 0, axis=1)
    df_CP['GCT_e_or_gamma_Select_3'] = df_CP.apply(lambda row: 1 if row['e_or_gamma_ET'] > MyConstants.c_GCT_3 else 0, axis=1)

    # FirstLayer, LastLayer, ShowerLen, CoreShowerLen
    ( df_CP['First_Layer'], df_CP['Last_Layer'], df_CP['Shower_Length'], df_CP['Core_Shower_Length']) = ShowerLengthPropertiesVectorized( df_CS['LayerBits'] )

    # ET_Weighted_Eta (send L1T the hyperbolic transform from mu(r/z) )
    mu_roz_single = Mean_coordinate_vectorized(df_CS['Wroz'], df_CS['W'] )
    mu_roz_lower_single = MyConstants.DoubleFloatToSingleFloat( MyConstants.roz_min_L1T / MyConstants.LSB_roz )
    mu_roz_upper_single = MyConstants.DoubleFloatToSingleFloat( MyConstants.roz_max_L1T / MyConstants.LSB_roz ) 
    mu_roz_sat = np.clip(mu_roz_single, mu_roz_lower_single, mu_roz_upper_single) # becomes numpy array here
    mu_roz_local_single = mu_roz_sat - mu_roz_lower_single
    mu_roz_local_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( mu_roz_local_single * MyConstants.c_roz_scaler_0 )
    mu_roz_local_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_roz_local_scaled, 20, 12 )
    mu_roz_int = MyRoundVectorized( mu_roz_local_fxp ).astype(int)
    mean_LUT_addr = np.clip(mu_roz_int, 0, 1023).astype(int)
    df_CP['ET_Weighted_Eta'] = np.array([dict_mu_eta_LUT[x] for x in mean_LUT_addr])

    # ET_Weighted_Phi
    mu_phi_single = Mean_coordinate_vectorized( df_CS['Wphi'], df_CS['W'] ) 
    mu_phi_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( mu_phi_single * MyConstants.c_phi_scaler )
    mu_phi_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_phi_scaled, 20, 12 )
    mu_phi_int = MyRoundVectorized( mu_phi_fxp ).astype(int)
    df_CP['ET_Weighted_Phi'] = mu_phi_int - MyConstants.c_Phi_Offset # Casting ET_Weighted_Phi to 'signed'

    # Saturated_Phi
    # Checking if Phi requires more than 9b and indicate with a flag. Optionally do a sign dependant saturate.
    et_weighted_phi = df_CP['ET_Weighted_Phi'].to_numpy()
    df_CP['Saturated_Phi'] = np.where(np.abs(et_weighted_phi) > 256, 1, 0)
    df_CP['ET_Weighted_Phi'] = np.clip(et_weighted_phi, -256, 255)

    # Nominal_Phi (1 if Phi is within the nominal Stage 2 120 deg sector)
    mask = (df_CP['ET_Weighted_Phi'] > -241) & (df_CP['ET_Weighted_Phi'] < 240)
    df_CP['Nominal_Phi'] = np.where(mask, 1, 0)

    # ET_Weighted_Z
    mu_z_single = Mean_coordinate_vectorized(df_CS['Wz'], df_CS['W'] )
    mu_z_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_z_single, 20, 12 )
    df_CP['ET_Weighted_Z'] = MyRoundVectorized( mu_z_fxp ).astype(int) # no need for scaling as LSB_TC = LSB_L1T

    # Number_of_Cells
    df_CP['Number_of_TCs'] = df_CS['N_TC']

    # Saturated_Trigger_Cell
    df_CP['Saturated_Trigger_Cell'] = df_CS['Sat_TC']

    # Quality_of_Fraction_in_CE_E
    df_CP['Quality_of_Fraction_in_CE_E'] = ((df_CS['E'] != MyConstants.c_Cluster_E_Sat) & (df_CS['E_EM'] != MyConstants.c_Cluster_E_Sat)).astype(int)

    # Quality_of_Fraction_in_core_CE_E
    df_CP['Quality_of_Fraction_in_core_CE_E'] = ((df_CS['E_EM_core'] != MyConstants.c_Cluster_E_Sat) & (df_CS['E_EM'] != MyConstants.c_Cluster_E_Sat)).astype(int)

    # Quality_of_Fraction_in_front_CE_H
    df_CP['Quality_of_Fraction_in_front_CE_H'] = ((df_CS['E'] != MyConstants.c_Cluster_E_Sat) & (df_CS['E_H_early'] != MyConstants.c_Cluster_E_Sat)).astype(int)

    # Quality_of_Sigmas_and_Means
    df_CP['Quality_of_Sigmas_and_Means'] = df_CS['ShapeQ']

    # Sigma_E
    sigma_E_single = Sigma_Energy_vectorized( df_CS['N_TC_W'], df_CS['W2'], df_CS['W'] )
    sigma_E_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_E_single * MyConstants.c_sigma_E_scaler )
    sigma_E_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_E_scaled, 20, 12 )
    df_CP['Sigma_E'] = MyRoundVectorized( sigma_E_fxp ).astype(int)

    # Sigma_ZZ
    sigma_zz_single = Sigma_Coordinate_vectorized( df_CS['W'], df_CS['Wz2'], df_CS['Wz'] )
    sigma_zz_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_zz_single * MyConstants.c_sigma_z_scaler )
    sigma_zz_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_zz_scaled, 20, 12 )
    sigma_zz_int = MyRoundVectorized( sigma_zz_fxp ).astype(int) # Rounding to L1T LSB
    df_CP['Sigma_ZZ'] = np.clip(sigma_zz_int, a_min=None, a_max=127)

    # Sigma_PhiPhi
    sigma_phiphi_single = Sigma_Coordinate_vectorized( df_CS['W'], df_CS['Wphi2'], df_CS['Wphi'] )
    sigma_phiphi_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_phiphi_single * MyConstants.c_sigma_phi_scaler )
    sigma_phiphi_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_phiphi_scaled, 20, 12 )
    sigma_phiphi_int = MyRoundVectorized( sigma_phiphi_fxp ).astype(int) # Rounding to L1T LSB
    df_CP['Sigma_PhiPhi'] = np.clip(sigma_phiphi_int, a_min=None, a_max=127)

    # Sigma_RoZRoZ
    sigma_rozroz_single = Sigma_Coordinate_vectorized( df_CS['W'], df_CS['Wroz2'], df_CS['Wroz'] )
    sigma_rozroz_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_rozroz_single * MyConstants.c_sigma_roz_scaler_1 )
    sigma_rozroz_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_rozroz_scaled, 20, 12 )
    sigma_rozroz_int = MyRoundVectorized( sigma_rozroz_fxp ).astype(int) # Rounding to L1T LSB
    df_CP['Sigma_RoZRoZ'] = np.clip(sigma_rozroz_int, a_min=None, a_max=127)

    # Sigma_EtaEta ( send the L1T the derivative based transform from sigma(r/z) )
    # mu_roz_single already calculated previously
    mu_roz_ds_lower_single = MyConstants.mu_roz_ds_lower
    mu_roz_ds_upper_single = MyConstants.mu_roz_ds_upper
    mu_roz_ds_sat = np.clip(mu_roz_single, mu_roz_ds_lower_single, mu_roz_ds_upper_single)
    mu_roz_ds_local_single = mu_roz_ds_sat - mu_roz_ds_lower_single
    mu_roz_ds_local_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( mu_roz_ds_local_single * MyConstants.c_roz_scaler_1 )
    mu_roz_ds_local_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_roz_ds_local_scaled, 20, 12 )
    mu_roz_ds_int = MyRoundVectorized( mu_roz_ds_local_fxp ).astype(int)
    mu_roz_addr = np.clip(mu_roz_ds_int, None, 63)

    # sigma_rozroz_single already calculated previously
    sigma_rozroz_ds_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_rozroz_single * MyConstants.c_sigma_roz_scaler_0 )
    sigma_rozroz_ds_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_rozroz_ds_scaled, 20, 12 )
    sigma_rozroz_ds_int = MyRoundVectorized( sigma_rozroz_ds_fxp ).astype(int) # Rounding to L1T LSB
    sig_roz_addr = np.clip(sigma_rozroz_ds_int, None, 63)
    sigma_eta_LUT_addr = mu_roz_addr * 64 + sig_roz_addr # 64 because sig_roz is 6 bits wide
    df_CP['Sigma_EtaEta'] = np.array([dict_sig_eta_LUT[x] for x in sigma_eta_LUT_addr]) # Perform the look-up
    
    return df_CP


def process_dataframe_parallel(df):
    # Split the input dataframe into smaller chunks
    num_chunks = mp.cpu_count()
    chunks = np.array_split(df, num_chunks)
    
    # Create a worker pool with the same number of processes as chunks
    pool = mp.Pool(num_chunks)
    
    # Map the chunks to the worker processes and combine the results
    results = pd.concat(pool.map(process_dataframe, chunks))
    
    # Close the worker pool and return the results
    pool.close()
    pool.join()
    return results

def core_function(label_id, sector_id, k):

    # # Read from a .parquet file
    # df_CS = pd.read_parquet('../data/EmulatorOutputs/Output_CS_Emulator_s180_{}_{}_k_{}.parquet'.format(sector_id, label_id, k), engine="pyarrow")

    # # Process the data
    # df_CP = process_dataframe_parallel(df_CS)

    # # Save to a .parquet file
    # df_CP.to_parquet('../data/EmulatorOutputs/Output_CP_Emulator_s180_{}_{}_k_{}.parquet'.format(sector_id, label_id, k), index = False)

    # # Describe the data frame (Debug)
    # print("Describing data frame...")
    # print((df_CP.describe().T).to_string())
    

    #TYPE tFieldType IS                  ( N_TC , E  , E_EM , E_EM_core , E_H_early , W   , N_TC_W , W2  , Wz  , Wroz , Wphi , Wz2 , Wroz2 , Wphi2 , LayerBits , Sat_TC , ShapeQ );
    
    N_TC = 0xf10
    E = 0x200000
    E_EM = 0x140000
    E_EM_core = 0x80000 
    E_H_early = 0x180000
    W = 0x100
    N_TC_W = 0
    W2 = 0
    Wz = 0xffff 
    Wroz = 0 
    Wphi = 0x1234
    Wz2 = 0x5678 
    Wroz2 = 0x9abc 
    Wphi2 = 0xdef0
    LayerBits = "1111111111111111110111111111000" # 0x7fffeff8#1111111111111111110111111111000 #2147479544#0x7fffeff8 
    Sat_TC = 1
    ShapeQ = 1
    print("N_TC : " + (hex(N_TC)))

    clus_E = MyRoundVectorized( E * MyConstants.c_ET_scaler ).astype(int) # Rounding to L1T LSB
    clus_e_or_gamma_ET = MyRoundVectorized( E_EM * MyConstants.c_ET_scaler ).astype(int)  # Rounding to L1T LSB
        
    # Energy Fractions
    clus_Fraction_in_CE_E = Energy_ratio_vectorized( E_EM, E, MyConstants.c_frac_scaler).astype(int)
    clus_Fraction_in_core_CE_E = Energy_ratio_vectorized( E_EM_core, E_EM, MyConstants.c_frac_scaler).astype(int)
    clus_Fraction_in_front_CE_H = Energy_ratio_vectorized( E_H_early , E , MyConstants.c_frac_scaler).astype(int)  

    First_Layer, Last_Layer, Shower_Length, Core_Shower_Length = ShowerLengthPropertiesVectorized( LayerBits )
    
    # ET_Weighted_Eta (send L1T the hyperbolic transform from mu(r/z) )
    mu_roz_single = Mean_coordinate_vectorized(Wroz, W )
    mu_roz_lower_single = MyConstants.DoubleFloatToSingleFloat( MyConstants.roz_min_L1T / MyConstants.LSB_roz )
    mu_roz_upper_single = MyConstants.DoubleFloatToSingleFloat( MyConstants.roz_max_L1T / MyConstants.LSB_roz ) 
    mu_roz_sat = np.clip(mu_roz_single, mu_roz_lower_single, mu_roz_upper_single) # becomes numpy array here
    mu_roz_local_single = mu_roz_sat - mu_roz_lower_single
    mu_roz_local_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( mu_roz_local_single * MyConstants.c_roz_scaler_0 )
    mu_roz_local_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_roz_local_scaled, 20, 12 )
    mu_roz_int = MyRoundVectorized( mu_roz_local_fxp ).astype(int)
    mean_LUT_addr = np.clip(mu_roz_int, 0, 1023).astype(int)
    #clus_ET_Weighted_Eta = np.array([dict_mu_eta_LUT[x] for x in mean_LUT_addr])
    clus_ET_Weighted_Eta = dict_mu_eta_LUT[mean_LUT_addr]
    
    # ET_Weighted_Phi
    mu_phi_single = Mean_coordinate_vectorized( Wphi, W ) 
    mu_phi_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( mu_phi_single * MyConstants.c_phi_scaler )
    mu_phi_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_phi_scaled, 20, 12 )
    mu_phi_int = MyRoundVectorized( mu_phi_fxp ).astype(int)
    ET_Weighted_Phi = mu_phi_int - MyConstants.c_Phi_Offset # Casting ET_Weighted_Phi to 'signed'
    
    # # Saturated_Phi
    # # Checking if Phi requires more than 9b and indicate with a flag. Optionally do a sign dependant saturate.
    # et_weighted_phi = df_CP['ET_Weighted_Phi'].to_numpy()
    # df_CP['Saturated_Phi'] = np.where(np.abs(et_weighted_phi) > 256, 1, 0)
    # df_CP['ET_Weighted_Phi'] = np.clip(et_weighted_phi, -256, 255)

    # # Nominal_Phi (1 if Phi is within the nominal Stage 2 120 deg sector)
    # mask = (df_CP['ET_Weighted_Phi'] > -241) & (df_CP['ET_Weighted_Phi'] < 240)
    # df_CP['Nominal_Phi'] = np.where(mask, 1, 0)

    # ET_Weighted_Z
    mu_z_single = Mean_coordinate_vectorized(Wz, W )
    mu_z_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_z_single, 20, 12 )
    clus_ET_Weighted_Z = MyRoundVectorized( mu_z_fxp ).astype(int) # no need for scaling as LSB_TC = LSB_L1T

    # Number_of_Cells
    clus_Number_of_TCs = N_TC

    # Saturated_Trigger_Cell
    clus_Saturated_Trigger_Cell = Sat_TC

    # Quality_of_Fraction_in_CE_E
    clus_Quality_of_Fraction_in_CE_E = ((E != MyConstants.c_Cluster_E_Sat) & (E_EM != MyConstants.c_Cluster_E_Sat))

    # Quality_of_Fraction_in_core_CE_E
    clus_Quality_of_Fraction_in_core_CE_E = ((E_EM_core != MyConstants.c_Cluster_E_Sat) & (E_EM != MyConstants.c_Cluster_E_Sat))

    # Quality_of_Fraction_in_front_CE_H
    clus_Quality_of_Fraction_in_front_CE_H = ((E != MyConstants.c_Cluster_E_Sat) & (E_H_early != MyConstants.c_Cluster_E_Sat))

    # Quality_of_Sigmas_and_Means
    clus_Quality_of_Sigmas_and_Means = ShapeQ

    # Sigma_E
    sigma_E_single = Sigma_Energy_vectorized( N_TC_W, W2, W )
    sigma_E_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_E_single * MyConstants.c_sigma_E_scaler )
    sigma_E_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_E_scaled, 20, 12 )
    clus_Sigma_E = MyRoundVectorized( sigma_E_fxp ).astype(int)

    # Sigma_ZZ
    sigma_zz_single = Sigma_Coordinate_vectorized( W, Wz2, Wz )
    sigma_zz_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_zz_single * MyConstants.c_sigma_z_scaler )
    sigma_zz_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_zz_scaled, 20, 12 )
    sigma_zz_int = MyRoundVectorized( sigma_zz_fxp ).astype(int) # Rounding to L1T LSB
    clus_Sigma_ZZ = np.clip(sigma_zz_int, a_min=None, a_max=127)

    # Sigma_PhiPhi
    sigma_phiphi_single = Sigma_Coordinate_vectorized( W, Wphi2, Wphi )
    sigma_phiphi_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_phiphi_single * MyConstants.c_sigma_phi_scaler )
    sigma_phiphi_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_phiphi_scaled, 20, 12 )
    sigma_phiphi_int = MyRoundVectorized( sigma_phiphi_fxp ).astype(int) # Rounding to L1T LSB
    clus_Sigma_PhiPhi = np.clip(sigma_phiphi_int, a_min=None, a_max=127)

    # Sigma_RoZRoZ
    sigma_rozroz_single = Sigma_Coordinate_vectorized( W, Wroz2, Wroz )
    sigma_rozroz_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_rozroz_single * MyConstants.c_sigma_roz_scaler_1 )
    sigma_rozroz_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_rozroz_scaled, 20, 12 )
    sigma_rozroz_int = MyRoundVectorized( sigma_rozroz_fxp ).astype(int) # Rounding to L1T LSB
    clus_Sigma_RoZRoZ = np.clip(sigma_rozroz_int, a_min=None, a_max=127)

    # Sigma_EtaEta ( send the L1T the derivative based transform from sigma(r/z) )
    # mu_roz_single already calculated previously
    mu_roz_ds_lower_single = MyConstants.mu_roz_ds_lower
    mu_roz_ds_upper_single = MyConstants.mu_roz_ds_upper
    mu_roz_ds_sat = np.clip(mu_roz_single, mu_roz_ds_lower_single, mu_roz_ds_upper_single)
    mu_roz_ds_local_single = mu_roz_ds_sat - mu_roz_ds_lower_single
    mu_roz_ds_local_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( mu_roz_ds_local_single * MyConstants.c_roz_scaler_1 )
    mu_roz_ds_local_fxp = MyConstants.SingleFloatToFixedPointVectorized( mu_roz_ds_local_scaled, 20, 12 )
    mu_roz_ds_int = MyRoundVectorized( mu_roz_ds_local_fxp ).astype(int)
    mu_roz_addr = np.clip(mu_roz_ds_int, None, 63)

    # sigma_rozroz_single already calculated previously
    sigma_rozroz_ds_scaled = MyConstants.DoubleFloatToSingleFloatVectorized( sigma_rozroz_single * MyConstants.c_sigma_roz_scaler_0 )
    sigma_rozroz_ds_fxp = MyConstants.SingleFloatToFixedPointVectorized( sigma_rozroz_ds_scaled, 20, 12 )
    sigma_rozroz_ds_int = MyRoundVectorized( sigma_rozroz_ds_fxp ).astype(int) # Rounding to L1T LSB
    sig_roz_addr = np.clip(sigma_rozroz_ds_int, None, 63)
    sigma_eta_LUT_addr = mu_roz_addr * 64 + sig_roz_addr # 64 because sig_roz is 6 bits wide
    clus_Sigma_EtaEta = dict_sig_eta_LUT[sigma_eta_LUT_addr] # Perform the look-up
    
    print("clus_E : " + (hex(clus_E)))
    print("clus_e_or_gamma_ET : " + (hex(clus_e_or_gamma_ET)))
    print("clus_Fraction_in_CE_E : " + (hex(clus_Fraction_in_CE_E)))
    print("clus_Fraction_in_core_CE_E : " + (hex(clus_Fraction_in_core_CE_E)))
    print("clus_Fraction_in_front_CE_H : " + (hex(clus_Fraction_in_front_CE_H)))
    # print("LayerBits : " + (hex(LayerBits)))
    # print("LayerBits : %d" %LayerBits)
    # print("LayerBits : " + (bin(LayerBits)))
    print("First_Layer : " + (hex(First_Layer)))
    print("Last_Layer : " + (hex(Last_Layer)))
    print("Shower_Length : " + (hex(Shower_Length)))
    print("Core_Shower_Length : " + (hex(Core_Shower_Length)))
    print("mu_roz_int : " + (hex(mu_roz_int)))
    print("mean_LUT_addr : " + (hex(mean_LUT_addr)))
    print("mean_LUT_addr :%d "%mean_LUT_addr)
    print("clus_ET_Weighted_Eta : " + (hex(clus_ET_Weighted_Eta)))
    # print("clus_ET_Weighted_Phi : " + (hex(clus_ET_Weighted_Phi)))
    print("clus_ET_Weighted_Z : " + (hex(clus_ET_Weighted_Z)))
    print("clus_Number_of_TCs : " + (hex(clus_Number_of_TCs)))
    print("clus_Saturated_Trigger_Cell : " + (hex(clus_Saturated_Trigger_Cell)))
    print("clus_Quality_of_Fraction_in_CE_E : " + (hex(clus_Quality_of_Fraction_in_CE_E)))
    print("clus_Quality_of_Fraction_in_core_CE_E : " + (hex(clus_Quality_of_Fraction_in_core_CE_E)))
    print("clus_Quality_of_Fraction_in_front_CE_H : " + (hex(clus_Quality_of_Fraction_in_front_CE_H)))
    print("clus_Quality_of_Sigmas_and_Means : " + (hex(clus_Quality_of_Sigmas_and_Means)))
    print("clus_Sigma_E : " + (hex(clus_Sigma_E)))
    print("clus_Sigma_ZZ : " + (hex(clus_Sigma_ZZ)))
    print("clus_Sigma_PhiPhi : " + (hex(clus_Sigma_PhiPhi)))
    print("clus_Sigma_RoZRoZ : " + (hex(clus_Sigma_RoZRoZ)))
    print("sigma_eta_LUT_addr : " + (hex(sigma_eta_LUT_addr)))
    print("clus_Sigma_EtaEta : " + (hex(clus_Sigma_EtaEta)))
    #print(dict_sig_eta_LUT)
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--label_id', type=str, help='Select sample id: pu, cp or ph', required=True)
    parser.add_argument('--sector_id', type=int, help='Select sector id: 0, 1, .., 5', required=True)
    parser.add_argument('--k', type=int, help='Specify the k parameter (positive integer) for the weight definition W')
    args = parser.parse_args()
    core_function(args.label_id, args.sector_id, args.k)
