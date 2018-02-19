for i in Fl_Type.h Fluid_Plugins.h Fluid_Image.h; do
	sed -f dll.sed < $i > tmp
	mv tmp $i
done
