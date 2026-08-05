### Example usage:
# cd ./data
# ../scriptrucio.sh IN2P3-CC_PHYS-SM filelist_xrootd.txt user.avallier.data17_13TeV.periodB.physics_Main.NTP_CC.grp23_v01_p4238_v26_outTree_CC_RecoZAi.root

echo ""
date
echo ""

rse=$1   # grid location, eg IN2P3-CC_PHYS-SM
ofile=$2 # name of the output txt file listing the root files (with xrootd protocol)
dsids=$3 # dataset identifier, eg user.avallier.data17_13TeV.periodB.physics_Main.NTP_CC.grp23_v01_p4238_v26_outTree_CC_RecoZAi.root

# Remove old file
if [[ -e $ofile ]];then
    rm $ofile
fi

rucio list-file-replicas --pfns --protocols root --rses "$rse" "$dsids" >> "${ofile}"

echo ""
echo "File list created: ${ofile}"
echo ""

# Create EOS folder for dataset
eos mkdir -p ${dsids}

echo "Created EOS folder: ${dsids}"

# Create symlinks from filelist
while IFS= read -r pfn; do
    if [[ -n "$pfn" && "$pfn" =~ root:// ]]; then
        basename=$(basename "$pfn")
        cd ${dsids}
        ln -s "$pfn" "$basename"
        echo "Created symlink: $basename -> $pfn"
    fi
done < "$ofile"

echo ""
echo "Symlinks created in ${dsids}/"
echo ""
date
echo ""
echo "C'est fini!"
echo ""