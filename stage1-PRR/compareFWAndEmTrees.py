import ROOT
import json

def compare_dictionaries(dict_1, dict_2, dict_1_name, dict_2_name, path=""):
    """Compare two dictionaries recursively to find non matching elements

    Args:
        dict_1: dictionary 1
        dict_2: dictionary 2

    Returns: string

    """
    err = ''
    key_err = ''
    value_err = ''
    old_path = path
    for k in dict_1:
        path = old_path + "[%s]" % k
        if not k in dict_2:
            key_err += "Key %s%s not in %s\n" % (dict_1_name, path, dict_2_name)
        else:
            if isinstance(dict_1[k], dict) and isinstance(dict_2[k], dict):
                err += compare_dictionaries(dict_1[k],dict_2[k],dict_1_name,dict_2_name, path)
            else:
                if dict_1[k] != dict_2[k]:
                    value_err += "Value of %s%s (%s) not same as %s%s (%s)\n"\
                        % (dict_1_name, path, dict_1[k], dict_2_name, path, dict_2[k])

    for k in dict_2:
        path = old_path + "[%s]" % k
        if not k in dict_1:
            key_err += "Key %s%s not in %s\n" % (dict_2_name, path, dict_1_name)

    return key_err + value_err + err



file_fw = ROOT.TFile.Open("FirmwareResultsMay12NoDelayNoPlayloop.root")
file_em = ROOT.TFile.Open("TCProcessor_EmulationResults.root")

tree_fw = file_fw.Get("Events")
tree_em = file_em.Get("Events")

fw_dict = {}
em_dict = {}
for evt in range(0,128):
    fw_dict[evt]={}
    em_dict[evt]={}
    for mod in range(0,300):
        fw_dict[evt][mod]={}
        em_dict[evt][mod]={}
        for column in range(0,10):
            fw_dict[evt][mod][column]=[]
            em_dict[evt][mod][column]=[]
        

for entry in tree_fw:
    if(entry.EventIn > -1):
       fw_dict[entry.EventIn][entry.Module][entry.Column].append((entry.Energy,entry.Address))

for entry in tree_em:
    em_dict[entry.Event][entry.Module][entry.Column].append((entry.Energy,entry.Address))


#print("FW dict")
#print(json.dumps(fw_dict,indent=4))

#print("EM dict")
#print(json.dumps(em_dict,indent=4))
print("In the printout below, the first index is the module number, the second the column number")
for evt in range(0,128):
    print("Comparing event ",evt)
    a = compare_dictionaries(fw_dict[evt],em_dict[evt],'FW dict','EM dict')
    print(a)
    print("==========================================================================================")

