"""
 Author: R. Shukla , IC London
 Contact: r.shukla@imperial.ac.uk

"""

class lword:
    def __init__(self, data=0, valid=0, bc0=0, start=0,last=0, strobe=1):
      self.data = data
      self.valid = valid
      self.bc0 = bc0
      self.start = start
      self.last = last
      self.strobe = strobe
    
    def print(self):
       print(f" StartOfOrbit = {self.bc0}, start = {self.start}, last = {self.last}, v = {self.valid}, strobe = {self.strobe}, D = 0x{self.data:016X}")

def string_to_lword_emp(s: str):
   # check legth of the string 
    
    if(len(s) == 20):
      dataStart = 4
      strobe = 1;
    elif (len(s)==21):
      dataStart = 5
      strobe = int(s[0],16)
    else:
      print(f"invalid string length for lword conversion, provided string = {s}")
      print(f"#Debug : provided string = {s}")
      exit(-1)
   
    valid = int(s[dataStart-1],16)
    last  = int(s[dataStart-2],16)
    start = int(s[dataStart-3],16)
    start_of_orbit = int(s[dataStart-4],16)
    data  = int(s[dataStart:],16)
   
    link_word = lword(data, valid, start_of_orbit, start, last, strobe)
    return link_word

# Returned number has n hex chars and does not have 0x   
def bin2hex(b , n):
  h = hex(int(b,2))[2:]  # striping 0x
  if len(h) < n:
    h = (n - len(h))*'0' + h
  return h
  
def hex2bin(h, nbits):
  return bin(int(h, 16))[2:].zfill(nbits)

def number_bitflip(n):
  n_bit = bin(n)[2:]   # remove leading 1b
  n_bit = n_bit.replace('0', '2')
  n_bit = n_bit.replace('1', '0')
  n_bit = n_bit.replace('2', '1')
  n_inv = int(n_bit, 2)
  return n_inv 

# assumeing 32 bit word  
def number_bitreverse(n):
  n_bit = bin(n)[2:]   # remove leading 1b
  # pad 0's
  n_bit = '0'*(32-len(n_bit)) + n_bit
  
  #n_bit_r = '0'*32
  #for i in range(32):
  #  n_bit_r[32-i] = n_bit[i]
  # Above will not work. need to use intermidiate character
  
  n_bit_r = n_bit[::-1]
  
  n_rev = int(n_bit_r, 2)
  return n_rev 


def read_emp_buffer(infile, nlinks):
  with open(infile, 'r') as f:
    buff = [ [] for _ in range(nlinks)]
    for l in f:
      if (l[:5] == 'Frame'):   # valid frame
        links = l.split()
        #print("--------------Split links into: ", len(links))  # Should be 75
        #print(links)
        # First 3 elements are names etc.
        # check if number of elements is correct before proceeding
        if (len(links) != nlinks + 3):
          print(" Incorrect file or number of links specified")
          exit(0)
        for i in range(3,nlinks+3):
          buff[i-3].append(links[i])
           
    #print("Size of link buffer : "+ str(len(buff)) + " x " + str(len(buff[0])))
    return buff

def read_emp_buffer_csp(infile, nlinks):
  with open(infile, 'r') as f:
    buff = [ [] for _ in range(nlinks)]
    debug_cnt =0
    for l in f:
      if (l[:5] == 'Frame'):   # valid frame
        links = l.split()
        #if debug_cnt <5:
          #print("--------------Split links into: ", len(links))  # Should be 75
          #print(links)
        # First 3 elements are names etc.
        # check if number of elements is correct before proceeding
        if (len(links) != (nlinks+1)*2):
          print(" Incorrect file or number of links specified")
          exit(0)
        for i in range(1,nlinks+1):
          buff[i-1].append((links[i*2]) + links[(i*2)+1] )
        debug_cnt += 1   
    #print("Size of link buffer : "+ str(len(buff)) + " x " + str(len(buff[0])))
    return buff

def read_emp_buffer_cspV2(infile, nlinks):
  with open(infile, 'r') as f:
    lID = f.readline()
    #print(lID)
    if(not(lID.startswith("ID:"))):
      print(f"Invalid line : {lID}")
      exit(0)
   
    lM = f.readline()
    #print(lM)
    if(not(lM.startswith("Metadata:"))):
      print(f"Invalid line : {lM}")
      exit(0)
    
    # 3rd line is empty
    lEmpty = f.readline()
    #print(lEmpty)
    if (not( lEmpty.strip() == "")):
      print(f"Invalid line : {lEmpty}; empty line expected here")
      exit(0)
    

    lLinkS = f.readline()
    #print(lLinkS)
    if(not(lLinkS.strip().startswith("Link"))):
      print(f"Invalid line : {lLinkS}")
      exit(0)
    
    lLink = lLinkS.split()
    lLink.remove("Link")
    print(f"Number of links are: {len(lLink)}")
    
    buff = [ [] for _ in range(len(lLink))]
    debug_cnt =0
    for l in f:
      if (l[:5] == 'Frame'):   # valid frame
        links = l.split()
        #if debug_cnt <5:
          #print("--------------Split links into: ", len(links))  # Should be 75
          #print(links)
        # First 3 elements are names etc.
        # check if number of elements is correct before proceeding
        if (len(links) != (nlinks+1)*2):
          print(" Incorrect file or number of links specified")
          exit(0)
        for i in range(1,nlinks+1):
          buff[i-1].append((links[i*2]) + links[(i*2)+1] )
        debug_cnt += 1   
    #print("Size of link buffer : "+ str(len(buff)) + " x " + str(len(buff[0])))
    buff_dict = {}
    for i in range(len(lLink)):
      buff_dict[int(lLink[i])] = buff[i]
    #print(buff_dict.keys())
    return buff_dict
  
"""
  returns dictonary of lword objects
  dictionary keys are actual link numbers (int) in the inout file i.e absolute
"""
def read_emp_buffer_cspV3(infile, nlinks):
  with open(infile, 'r') as f:
    lID = f.readline()
    #print(lID)
    if(not(lID.startswith("ID:"))):
      print(f"Invalid line : {lID}")
      exit(0)
   
    lM = f.readline()
    #print(lM)
    if(not(lM.startswith("Metadata:"))):
      print(f"Invalid line : {lM}")
      exit(0)
    
    # 3rd line is empty
    lEmpty = f.readline()
    #print(lEmpty)
    if (not( lEmpty.strip() == "")):
      print(f"Invalid line : {lEmpty}; empty line expected here")
      exit(0)
    

    lLinkS = f.readline()
    #print(lLinkS)
    if(not(lLinkS.strip().startswith("Link"))):
      print(f"Invalid line : {lLinkS}")
      exit(0)
    
    lLink = lLinkS.split()
    lLink.remove("Link")
    print(f"Number of links are: {len(lLink)}")
    
    buff = [ [] for _ in range(len(lLink))]
    debug_cnt =0
    for l in f:
      if (l[:5] == 'Frame'):   # valid frame
        links = l.split()
        #if debug_cnt <5:
          #print("--------------Split links into: ", len(links))  # Should be 75
          #print(links)
        # First 3 elements are names etc.
        # check if number of elements is correct before proceeding
        if (len(links) != (nlinks+1)*2):
          print(" Incorrect file or number of links specified")
          exit(0)
        for i in range(1,nlinks+1):
          buff[i-1].append( string_to_lword_emp((links[i*2]) + links[(i*2)+1]) )
        debug_cnt += 1   
    #print("Size of link buffer : "+ str(len(buff)) + " x " + str(len(buff[0])))
    buff_dict = {}
    for i in range(len(lLink)):
      buff_dict[int(lLink[i])] = buff[i]
    #print(buff_dict.keys())
    return buff_dict

def lpgbt_to_econt(lpgbt_in):
  if len(lpgbt_in) == 0:
    print("warning: lpgbt_to_econt : lpgbt_in list is empty")  
  
  e0= list()
  e1= list()
  e2= list()
  e3= list()
  e4= list()
  e5= list()
  e6= list()
  
  i=0
  nlword = 0
  #print(f"Debug: Length of lpgbt buffer : {len(lpgbt_in)}")
  lword_hist = "1v0000000000000000"
  while(nlword+7 < len(lpgbt_in)):
    #print(f"Debug: At nlword = {nlword} ; current lword = {lpgbt_in[nlword]} ; host = {lword_hist}")
    if lpgbt_in[nlword].startswith('1v') and lword_hist.startswith('0v'):
      # start of the lpgbt frame
      #print(f"debug: Start of lpgbt frame at nlword = {nlword}; lword = {lpgbt_in[nlword]}")
      e0.append(int(lpgbt_in[nlword][2:], 16))  # Remove valid flag and parse hex
      e1.append(int(lpgbt_in[nlword+1][2:], 16))
      e2.append(int(lpgbt_in[nlword+2][2:], 16))
      e3.append(int(lpgbt_in[nlword+3][2:], 16))
      e4.append(int(lpgbt_in[nlword+4][2:], 16))
      e5.append(int(lpgbt_in[nlword+5][2:], 16))
      e6.append(int(lpgbt_in[nlword+6][2:], 16))
      lword_hist = lpgbt_in[nlword+7]  
      nlword += 8
    else:
      lword_hist = lpgbt_in[nlword]
      nlword += 1
      
  
  lpgbt_econt = list()
  
  lpgbt_econt.append(e0)
  lpgbt_econt.append(e1)
  lpgbt_econt.append(e2)
  lpgbt_econt.append(e3)
  lpgbt_econt.append(e4)
  lpgbt_econt.append(e5)
  lpgbt_econt.append(e6)
  
  return lpgbt_econt  


def lpgbt_to_econt_csp(lpgbt_in):
  if len(lpgbt_in) == 0:
    print("warning: lpgbt_to_econt : lpgbt_in list is empty")  
    return -1

  buff = [ [] for _ in range(7)]  # lpgbt can have only 7 elinks
  #valid_hist = '0' 
  nvalid = 0
  nlword = 0
  #print(f"Debug: Length of lpgbt buffer : {len(lpgbt_in)}")
  # find first valid i.e. start (invalid to valid edge); not relying on start
  
  if(len(lpgbt_in[nlword]) == 21):
    valid_hist =  lpgbt_in[nlword][4]
  elif (len(lpgbt_in[nlword]) == 20):
    valid_hist =  lpgbt_in[nlword][3]
    
  
  nlword +=1
  while(nlword < len(lpgbt_in)):
    if(len(lpgbt_in[nlword]) == 21):   # meta data for stobe exists
      if( valid_hist == '0' and lpgbt_in[nlword][4] == '1') :  # valid_hist == '0'  lpgbt_in[nlword][1] == '2' 
        break;
      else:
        valid_hist =  lpgbt_in[nlword][4]
      
    if(len(lpgbt_in[nlword]) == 20):   # meta data for stobe exists
      if( valid_hist == '0' and lpgbt_in[nlword][3] == '1') :   #valid_hist == '0' and   lpgbt_in[nlword][1] == '1'
        break;
      else:
        valid_hist =  lpgbt_in[nlword][3]
    nlword +=1
  
  #print(f"#Debug: Start found at {nlword} = {lpgbt_in[nlword]}")
  #print(f"#Debug: last word should be End of Packet:  {lpgbt_in[nlword-1]}")

  while(nlword+7 < len(lpgbt_in)):
    #print(f"Debug: At nlword = {nlword} ; current lword = {lpgbt_in[nlword]}")
    #print(f"#Debug: last word should be End of Packet:  {lpgbt_in[nlword-1]}")

    for nelink in range(0,7): 
      if len(lpgbt_in[nlword]) == 21:   # strobe is not always asserted, check for valid strobe 
        if lpgbt_in[nlword+nelink][0] == '1' and lpgbt_in[nlword+nelink][4] == '1' : # valid and strobe
          buff[nelink].append(int(lpgbt_in[nlword+nelink][5:], 16))
          #print(f"Found valid word for elink {nelink}")
          nvalid +=1
      elif len(lpgbt_in[nlword]) == 20:   # strobe is  always asserted 
        if lpgbt_in[nlword+nelink][3] == '1' : # valid
          buff[nelink].append(int(lpgbt_in[nlword+nelink][4:], 16))
          #print(f"Found valid word for elink {nelink}")
          nvalid +=1
    nlword += 8

  print(f"INFO: total frames processed : {nlword}, valid frames were {nvalid}")  
  return buff  

def read_zcu_file_v3(infile, nelinks):
  with open(infile, 'r') as f:
    buff = [ [] for _ in range(nelinks)]
    nlines = 0
    for l in f:
      if (nlines != 0):
        elink_l = l.split(',')
        if (len(elink_l) != nelinks):
          print(" ZCU input file format error ")
          exit(-1)
        else:
          for i in range(nelinks):
            buff[i].append(int(elink_l[i],16))
      
      nlines += 1
      
  return buff

'''
  def align_elink(elink, lshift s, r, w )
  elink: elink data array (list)
  lshift : Bit-shift direction, left or right (bool)
  s : bit shift (int 0-15)
  r : reverse bit orientation in the word (bool)
  w : enable word shift (bool)
  
  returns modified elink array of size len(elink)-1
'''
def align_elink(elink, lshift, s, r, w ):
  elink_mod = list()
  buff = 0
  nwords = 0
  elink_word_p =0
  buff_w_bin = '0'*32
  #lshift = True
  #print(f"Shift = {s}, reverse = {r}, word_shift = {w}") 
  for elink_word in elink:
    #if nwords < 5:
    # print(f"Processing word {nwords} = {hex(elink_word)}")
    #if(not r):
    if(lshift):
      if (not r):
        elink_word_p = elink_word
      else:
        elink_word_p = number_bitreverse(elink_word)
      
      if (nwords == 0):
        buff = elink_word_p
        buff_bin = bin(buff)[2:].zfill(32)
        buff_bin = buff_bin[s:]
      else:
        # do the allignment - left shift
        in_bin = bin(elink_word_p)[2:].zfill(32)
        out_bin = buff_bin + in_bin[:s]
        buff_bin = in_bin[s:]
        if (w and not(r)):
          out_bin_w =  buff_w_bin[-16:] + out_bin[:16]
          buff_w_bin = out_bin
        elif (w and r):
          out_bin_w =  out_bin[-16:] + buff_w_bin[:16]  # probably needs a review
          buff_w_bin = out_bin  
    else:
      if (not r):
        elink_word_p = elink_word
      else:
        elink_word_p = number_bitreverse(elink_word)
      #print(hex(elink_word_p))
      if (nwords == 0):
        buff = elink_word_p
        buff_bin = bin(buff)[2:].zfill(32)
        buff_bin = buff_bin[:-s]
      else:
        #print(f"When nwords > 0 , {hex(elink_word_p)}")
        # do the allignment - right shift
        in_bin = bin(elink_word_p)[2:].zfill(32)
        out_bin = in_bin[-s:] + buff_bin
        buff_bin = in_bin[:-s]     
        if (w and r):
          out_bin_w =  out_bin[-16:] + buff_w_bin[:16] 
          buff_w_bin = out_bin
        elif (w and not(r)):
          out_bin_w =  buff_w_bin[-16:] + out_bin[:16]
          buff_w_bin = out_bin 
           
    if (w and nwords > 1):
      elink_mod.append(int(out_bin_w,2))
    elif (not(w) and nwords > 0):
      elink_mod.append(int(out_bin,2))
    
    nwords += 1        
  
  return elink_mod



def main():
  # test
  ''' 
  infile = '../data/ZCU/temp_0_TC.csv'
  rx_buff = read_zcu_file_v3(infile,7)
  print(len(rx_buff))
  print(len(rx_buff[0]))
  
  print_elink = 1
  for i in range (5):
    print(hex(rx_buff[print_elink][i]))
    
  e1_aligned = align_elink(rx_buff[print_elink], False, 13, True, False) 
  for i in range (5):
    print(hex(e1_aligned[i]))
  
  
  '''
  #infile = '../data/ZTC_271022_rx_summary_BX_000_02.txt'
  #infile = '../data/PRBS_281022_rx_summary_BX_00_00.txt'
  
  #infile = '../data/ZTC_311022_rx_summary_BX_2200_00.txt'
  
  #emp_buff = read_emp_buffer(infile,3)
  #rx_buff = lpgbt_to_econt(emp_buff[1])
   
  #print(len(rx_buff))
  #print(len(rx_buff[0]))
  
  #print_elink = 6
  #for i in range (5):
  #  print(f"{rx_buff[print_elink][i]:08x}")
  
  #print("-Aligned-")  
  #e1_aligned = align_elink(rx_buff[print_elink], False, 11, True, False) 
  #for i in range (5):
  #  print(f"{e1_aligned[i]:08x}")
  
  #for firmware simulations - print 10 lpgbt frames
  '''
  for i in range (10):
      for j in range(7):  
          if (j != 6):
              print(f"{rx_buff[j][i+3]:08x}",end="")
          else:
              print(f"{rx_buff[j][i+3]:08x}")
      #print("\n")
  '''
  
  ## CSP format test
  infile = 'tmux_lite_emp_buffer_test/tx_summary_noRst_1.txt'
  l0 = read_emp_buffer_cspV3(infile, 108)

  print(f"Type of returned object = {type(l0)}")
  print(f"Size of the read link dict = {len(l0)} ")
  
  keys_list = list(l0.keys())
  print("Key list:")
  print(keys_list)
  
  print(f"\nLink word object type {type(l0[keys_list[0]][0])}")
  
  print(f" Len of last link = {len(l0[111])}")

  # example file has link numbers from 4
  for linkKey in keys_list:
    i =0
    print(f" Link :{linkKey}")
    for frame in l0[linkKey]:
      if i < 2:
        frame.print()
      i+=1
  
  # elinks = lpgbt_to_econt_csp(l0[0])
  # print(f"Size of elinks buffer is {len(elinks)}")

  # for elink in elinks:
  #   print(len(elink))
  #   i =0
  #   for dword in elink:
  #     if i < 5:
  #       print(hex(dword))
  #     i+=1

if __name__ == '__main__':
  main()

