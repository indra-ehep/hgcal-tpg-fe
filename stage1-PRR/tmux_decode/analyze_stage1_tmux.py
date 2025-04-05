"""
 Author: R. Shukla , IC London
 Contact: r.shukla@imperial.ac.uk

"""

import argparse
from emp_buffer_utils import read_emp_buffer_cspV3, lword



def inspect_link(alinkdata, alinkgroup, aexp_packet_start=0):
    #start_word, end_word = 0, 0
    iword = 0
    in_packet = False
    npacket = 0
    err_packet_len = 0
    err_tc = 0
    err_tsum = 0
    tc = []
    tsum = []
    bx_ids = []
    err_bx_mismatch = 0
    err_bx_gap = 0
    bx_id_prev = 0
    err_pkt_start_index = 0

    for nword, llword in enumerate(alinkdata):
        #if (nword < 300):
        #   llword.print()
        if (llword.start == 1 and llword.valid == 1):
          # packet has started
          if (in_packet): # last packet has ended
            npacket += 1
            if(iword != 108):
              # Not counting invalid words
              print(f"Error in packet lenth; expected {108} but saw {iword}, npacket = {npacket}, at word = {nword}")
              err_packet_len+=1
        #   elif (npacket == 0):
        #       #first packet on the link
        #       if(aexp_packet_start != 0 and nword != aexp_packet_start):
        #          err_pkt_start_index +=1
        #          if (nword - aexp_packet_start >=  144): 
        #             # one full old packet can fit there, so we must have detected old packet; has been happening on Link14
        #             # ignore this packet  
          in_packet = True
          iword = 0
        
        if(in_packet and llword.valid == 1):
            # disssemble packet
            hdr = (llword.data & 0x7000000000000000) >> 61
            tsum_mask = 0x1FE0000000000000
            tc_mask = 0x00001FFFC0000000

            for n in range(2):
              tsum_temp = (llword.data & (tsum_mask >> (n*8))) >> (53 - n*8)
              tsum.append(tsum_temp)
              tsum_exp = ((alinkgroup * 108* 2) + (iword*2) + n) & 0xFF
              if(iword == 0 and n == 0):
                tsum_bx_id = tsum_temp
                continue
              if(tsum_temp != tsum_exp):
                err_tsum += 1
                print(f" TSUM error at packet = {npacket}, at word = {iword}, pTT = {n}, global word = {nword}; expected = {tsum_exp:X}, found = {tsum_temp:X}  ")
                llword.print()
            
        
            for n in range(3):
              tc_temp = (llword.data & (tc_mask >> (n*15))) >> (30 - 15*n)
              tc.append(tc_temp)
              tc_hdr = (tc_temp >> 12) & 0x7
              if(iword == 0 and n == 0):
                # LSb 8 bits of BX-IDs should match
                if ((tc_temp & 0xFF) != tsum_bx_id):
                    err_bx_mismatch+=1

                if((tc_temp - bx_id_prev) < 0):   # BCR
                   bx_gap = 3564 - bx_id_prev + tc_temp
                else:
                   bx_gap =  tc_temp - bx_id_prev                                
                if(bx_gap != 18 and npacket != 0):
                    err_bx_gap += 1
                    print(f" BX Gap (18) error at packet = {npacket}; bx-id = {tc_temp}, bx_id_prev = {bx_id_prev}")
                
                bx_ids.append(tc_temp)
                bx_id_prev = tc_temp
                continue
              tc_exp = (0x5 << 12) | ( ( (alinkgroup * 108* 3) +  (iword*3) + n) & 0xFFF) 
              if(tc_temp  != tc_exp):
                err_tc += 1
                print(f" TC error at packet = {npacket}, at word = {iword}, TC = {n} , global word = {nword}; expected = {tc_exp:X}, found = {tc_temp:X} ")

            iword += 1

    print(f" nPackets = {npacket}")
    print(" --- Errors:  --" )
    print(f" err_packet_len = {err_packet_len} ")
    print(f" err_tc = {err_tc}")
    print(f" err_tsum = {err_tsum}")
    print(f" err_bx_mismatch = {err_bx_mismatch}")
    print(f" err_bc gap = {err_bx_gap}")
    print("---------------------------\n")
    return bx_ids


def inspect_link_stage1(alinkdata, alinkgroup, aexp_packet_start=0):
    #start_word, end_word = 0, 0
    iword = 0
    in_packet = False
    npacket = 0
    err_packet_len = 0
    err_tc = 0
    err_tsum = 0
    tc = []
    tsum = []
    bx_ids = []
    err_bx_mismatch = 0
    err_bx_gap = 0
    bx_id_prev = 0
    err_pkt_start_index = 0

    for nword, llword in enumerate(alinkdata):
        #if (nword < 300):
        #   llword.print()
        if (llword.start == 1 and llword.valid == 1):
          # packet has started
          if (in_packet): # last packet has ended
            npacket += 1
            if(iword != 108):
              # Not counting invalid words
              print(f"Error in packet lenth; expected {108} but saw {iword}, npacket = {npacket}, at word = {nword}")
              err_packet_len+=1
        #   elif (npacket == 0):
        #       #first packet on the link
        #       if(aexp_packet_start != 0 and nword != aexp_packet_start):
        #          err_pkt_start_index +=1
        #          if (nword - aexp_packet_start >=  144): 
        #             # one full old packet can fit there, so we must have detected old packet; has been happening on Link14
        #             # ignore this packet  
          in_packet = True
          iword = 0
        
        if(in_packet and llword.valid == 1):
            # disssemble packet
            hdr = (llword.data & 0x7000000000000000) >> 61
            tsum_mask = 0x1FE0000000000000
            tc_mask = 0x00001FFFC0000000
            
            # this build has BX ID only in tower sum
            for n in range(2):
              tsum_temp = (llword.data & (tsum_mask >> (n*8))) >> (53 - n*8)
              tsum.append(tsum_temp)
              tsum_exp = ((alinkgroup * 108* 2) + (iword*2) + n) & 0xFF
              if(iword == 0 and n == 0):
                tsum_bx_id = tsum_temp
                # its going to roll over at 255 as well so that needs to be checked as well
                # to do
                if((tsum_temp - bx_id_prev) < 0):   # BCR
                   bx_gap = 3564 - bx_id_prev + tsum_temp
                else:
                   bx_gap =  tsum_temp - bx_id_prev                                
                if(bx_gap != 18 and npacket != 0):
                    err_bx_gap += 1
                    print(f" BX Gap (18) error at packet = {npacket}; bx-id = {tsum_temp}, bx_id_prev = {bx_id_prev}")
                
                bx_ids.append(tsum_temp)
                bx_id_prev = tsum_temp
                
                continue
              if(tsum_temp != tsum_exp):
                err_tsum += 1
                print(f" TSUM error at packet = {npacket}, at word = {iword}, pTT = {n}, global word = {nword}; expected = {tsum_exp:X}, found = {tsum_temp:X}  ")
                llword.print()
            
        
            for n in range(3):
              tc_temp = (llword.data & (tc_mask >> (n*15))) >> (30 - 15*n)
              tc.append(tc_temp)
              #tc_hdr = (tc_temp >> 12) & 0x7
              # no checking for TCs
              #tc_exp = (0x5 << 12) | ( ( (alinkgroup * 108* 3) +  (iword*3) + n) & 0xFFF) 
              #if(tc_temp  != tc_exp):
              #  err_tc += 1
              #  print(f" TC error at packet = {npacket}, at word = {iword}, TC = {n} , global word = {nword}; expected = {tc_exp:X}, found = {tc_temp:X} ")

            iword += 1
    """
    print(f" nPackets = {npacket}")
    print(" --- Errors:  --" )
    print(f" err_packet_len = {err_packet_len} ")
    print(f" err_tc = {err_tc}")
    print(f" err_tsum = {err_tsum}")
    print(f" err_bx_mismatch = {err_bx_mismatch}")
    print(f" err_bc gap = {err_bx_gap}")
    print("---------------------------\n")
    """
    return bx_ids, tc, tsum


def inpect_bx_ids(bx_id_array):
    # check BX IDs
    # Last link should have least packets
    bx_id_prev = 0
    err_bx_gap = 0
    for i in range(len(bx_ids_array[17])):
       for j in range(18):
          if (i > len(bx_ids_array[j]) - 1):
            #no more packet in the link
            continue 
          #print(bx_ids_array[j][i]) # print at the end - corrected value
          if( not ( i ==0 and j== 0)):
            if (bx_ids_array[j][i] - bx_id_prev != 1):
                #print(f"#Debug : BX-id_prev = {bx_id_prev}, bx-id current  = {bx_ids_array[j][i]}")
                if ((bx_id_prev == 3563 and bx_ids_array[j][i] == 0)): 
                    # BCR
                    #print(f"#Debug : BCR detected at link {j}, napcket {i}, BX-id_prev = {bx_id_prev}, bx-id current  = {bx_ids_array[j][i]}")
                    continue
                elif ( (bx_ids_array[j][i+1]  - bx_ids_array[j][i]) == 18 or (3564 - bx_ids_array[j][i] + bx_ids_array[j][i+1]) == 18):
                   #if the next packet on the same link is exactly 18 BX apart, it means there was a full valid older packet on the link
                   # Just need to ignore this
                   # pop that entry from the link; 
                   # another check would be that this link will have 1 packet more than others : not always true
                   # has been happening on link 14 onwards
                   # this should happen only for i== 0
                   #print("Debug: old packet case")
                   ####if(len(bx_ids_array[j]) > len(bx_ids_array[j-1])): # this does not happen always as after couple of ticks, latest packet will go out of buffer
                   #print(f"#Debug : link = {j}, npacket = {i}")
                   #print(f"#Debug : Old paceket detected on link {j}, rechecking BX-id difference ignoring this entry and removing the BX-id entry for future comparisons")
                   #print(f"#Debug : removing entry = {bx_ids_array[j][i]}, next (hopefully correct entry) is = {bx_ids_array[j][i+1]}")
                   bx_ids_array[j].pop(i)
                   #print(f"#Debug: after removing = now current entry is {bx_ids_array[j][i]}")
                   # redo the comparison : this code arrangement is not ideal since code below is repeated but, in interest of saving time its fine
                   if (bx_ids_array[j][i] - bx_id_prev != 1):
                       if (not (bx_id_prev == 3563 and bx_ids_array[j][i] == 0)): # BCR
                          print(f"BX gap more than 1 BX at link = {j}, packet= {i} ; BX_id ={bx_ids_array[j][i]}, BX-id-prev = {bx_id_prev}" )
                          err_bx_gap += 1
                else:
                    print(f"BX gap more than 1 BX at link = {j}, packet= {i} ; BX_id ={bx_ids_array[j][i]}, BX-id-prev = {bx_id_prev}" )
                    err_bx_gap += 1
                       
          else:
             print(f"first bx = {bx_ids_array[j][i]}")
             #print(f"Link; npacket; BX_id")  
          
          #print(bx_ids_array[j][i])
          #print(f" Link {j}, packet {i} , BX ID = {bx_ids_array[j][i]}")
          #print(f"{j},{i},{bx_ids_array[j][i]}")
    
          bx_id_prev = bx_ids_array[j][i]
    
    return err_bx_gap

if __name__ == "__main__":
    
    all_args = argparse.ArgumentParser()

    # Add arguments to the parser
    all_args.add_argument("-if", "--in_file", type=str, required=True, default="data/tx_summary.txt",
    help="EMP TX summary capture file")
    
    all_args.add_argument("-oname", "--out_file_name", type=str, required=False, default="out",
    help="Extracted TCs from specified link")

    all_args.add_argument("--nlinks_in", type=int, required=False, default = 108,
    help="Number of links in TX capture file. This will be auto-detected in next version.")

    all_args.add_argument("--ndump", type=int, required=False, default = 0,
    help="Number of the link, for which TC and pTTs to be dumped to a file.")
   
    args = all_args.parse_args()

    #"s1_build_data/RX_60_pairs_ouput_orderV1/data_20250207_3/tx_summary.txt"
    linkdata_array = read_emp_buffer_cspV3(args.in_file, args.nlinks_in)
    link_keys = list(linkdata_array.keys())
    #print(len(link_keys))
    #print(link_keys)

    # Output link order V1
    # TMUX 0 : 16:27, 100:105
    # TMUX 1 : 106:123
    # TMUX 2 : 28:45
    # TMUX 3 : 46:63
    # TMUX 4 : 64:81
    # TMUX 5 : 82:99

    # output file names
    tc_out_name = args.out_file_name + "_tc.txt"
    pTT_out_name = args.out_file_name + "_pTT.txt" 

    link_keys_tmux0 = [x for x in range(16,28)] + [x for x in range(100,106)] 
    link_keys_tmux5 = [x for x in range(82,100)]
    link_data_list = []
    link_data_list = []
    bx_ids_array = []
    ntmux = 0

    # copy 18 links
    for i in range(18):
      link_data_list.append(linkdata_array[link_keys_tmux0[i]])

    
    print(f" Checking TMUX {ntmux}")
    for nlink, linkdata in enumerate(link_data_list):
      print(f" Checking link : {nlink} : ")
      bx_ids, tc, tsum = inspect_link_stage1(linkdata, ntmux)
      bx_ids_array.append(bx_ids)
      if(nlink==args.ndump):
        print(f" Dumping link : {nlink} : ")
        #dump TC data
        with open(tc_out_name, "w") as fw:
          fw.write(f"NTC\tTC-label\tTC-Energy\n") 
          for ntc, tc_data in enumerate(tc):
            # TC processor swaps E and Lable field
            # hex print
            #print(f" {ntc}  {((tc_data & 0x7e00) >> 9):02x}   { (tc_data & 0x1FF):03x} ")
            #print(f" {ntc}  {((tc_data & 0x7e00) >> 9):03d}   { (tc_data & 0x1FF):03d} ") 
            fw.write(f"{ntc}\t{((tc_data & 0x7e00) >> 9):03d}\t{ (tc_data & 0x1FF):03d}\n") 

        
        with open(pTT_out_name, "w") as fw:
          fw.write(f"n_pTT\tpTT\n") 
          for nptt, pTT in enumerate(tsum):
            fw.write(f"{nptt}\t{((pTT & 0xFF)):03d}\n") 
       
              
              
    """
       When link mapping is continious
    """
    """
    for ntmux in range(6):
      # exiting function deals with group of 18 links
      # so its easier to make the group of 18 links
      link_data_list = []
      bx_ids_array = []
      # copy 18 links
      for i in range(18):
         link_data_list.append(linkdata_array[link_keys[(18*ntmux) + i]])
      
      print(f" Checking TMUX = {ntmux}")
      for nlink, linkdata in enumerate(link_data_list):
         print(f" Checking link : {nlink} : ")
         bx_ids, tc, tsum = tageinspect_link_s1(linkdata, 5-ntmux)
         bx_ids_array.append(bx_ids)
         if(nlink==0):
            #dump TC data
            for ntc, tc_data in enumerate(tc):
               print(f" {ntc}  {((tc_data & 0x7e00) >> 9):02x}   { (tc_data & 0x1FF):03x} ")
    """
      # err_bx_gap = inpect_bx_ids(bx_ids_array)
      # print(f" BX gap error = {err_bx_gap}")
    
    # # check BX IDs
    # # Last link should have least packets
    # bx_id_prev = 0
    # err_bx_gap = 0
    # for i in range(len(bx_ids_array[17])):
    #    for j in range(18):
    #       if (i > len(bx_ids_array[j]) - 1):
    #         #no more packet in the link
    #         continue 
    #       #print(bx_ids_array[j][i]) # print at the end - corrected value
    #       if( not ( i ==0 and j== 0)):
    #         if (bx_ids_array[j][i] - bx_id_prev != 1):
    #             #print(f"#Debug : BX-id_prev = {bx_id_prev}, bx-id current  = {bx_ids_array[j][i]}")
    #             if ((bx_id_prev == 3563 and bx_ids_array[j][i] == 0)): 
    #                 # BCR
    #                 #print(f"#Debug : BCR detected at link {j}, napcket {i}, BX-id_prev = {bx_id_prev}, bx-id current  = {bx_ids_array[j][i]}")
    #                 continue
    #             elif ( (bx_ids_array[j][i+1]  - bx_ids_array[j][i]) == 18 or (3564 - bx_ids_array[j][i] + bx_ids_array[j][i+1]) == 18):
    #                #if the next packet on the same link is exactly 18 BX apart, it means there was a full valid older packet on the link
    #                # Just need to ignore this
    #                # pop that entry from the link; 
    #                # another check would be that this link will have 1 packet more than others : not always true
    #                # has been happening on link 14 onwards
    #                # this should happen only for i== 0
    #                #print("Debug: old packet case")
    #                ####if(len(bx_ids_array[j]) > len(bx_ids_array[j-1])): # this does not happen always as after couple of ticks, latest packet will go out of buffer
    #                #print(f"#Debug : link = {j}, npacket = {i}")
    #                #print(f"#Debug : Old paceket detected on link {j}, rechecking BX-id difference ignoring this entry and removing the BX-id entry for future comparisons")
    #                #print(f"#Debug : removing entry = {bx_ids_array[j][i]}, next (hopefully correct entry) is = {bx_ids_array[j][i+1]}")
    #                bx_ids_array[j].pop(i)
    #                #print(f"#Debug: after removing = now current entry is {bx_ids_array[j][i]}")
    #                # redo the comparison : this code arrangement is not ideal since code below is repeated but, in interest of saving time its fine
    #                if (bx_ids_array[j][i] - bx_id_prev != 1):
    #                    if (not (bx_id_prev == 3563 and bx_ids_array[j][i] == 0)): # BCR
    #                       print(f"BX gap more than 1 BX at link = {j}, packet= {i} ; BX_id ={bx_ids_array[j][i]}, BX-id-prev = {bx_id_prev}" )
    #                       err_bx_gap += 1
    #             else:
    #                 print(f"BX gap more than 1 BX at link = {j}, packet= {i} ; BX_id ={bx_ids_array[j][i]}, BX-id-prev = {bx_id_prev}" )
    #                 err_bx_gap += 1
                       
    #       else:
    #          print(f"first bx = {bx_ids_array[j][i]}")
    #          #print(f"Link; npacket; BX_id")  
          
    #       #print(bx_ids_array[j][i])
    #       #print(f" Link {j}, packet {i} , BX ID = {bx_ids_array[j][i]}")
    #       #print(f"{j},{i},{bx_ids_array[j][i]}")
    
    #       bx_id_prev = bx_ids_array[j][i]
          
    
    #print(f" BX gap error = {err_bx_gap}")
