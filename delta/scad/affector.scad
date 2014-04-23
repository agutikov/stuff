

H = 10;

R = 30;

thikness = 10;

base = R + thikness;

l = base*sqrt(3)/3;
L = sqrt(l*l + base*base);


r = 2;
r_base = R + thikness/2;
r_l = r_base/2;
r_L = sqrt(r_base*r_base - r_l*r_l);

r_bolt = 1.5;

difference() {
	
	// 6-angle 
	linear_extrude(height = H) {
		polygon(points=[ [base, -l], [base, l], [0, L], [-base, l], [-base, -l], [0, -L] ]);
	
	}
	{
		// Hole in center
		cylinder(h = H, r = R);

		translate([r_base, 0, 0]) cylinder(h = H, r = r);
		translate([-r_base, 0, 0]) cylinder(h = H, r = r);

		translate([-r_L, r_l, 0]) cylinder(h = H, r = r);
		translate([-r_L, -r_l, 0]) cylinder(h = H, r = r);

		translate([-r_l, r_L, 0]) cylinder(h = H, r = r);
		translate([-r_l, -r_L, 0]) cylinder(h = H, r = r);

		translate([r_L, r_l, 0]) cylinder(h = H, r = r);
		translate([r_L, -r_l, 0]) cylinder(h = H, r = r);

		translate([r_l, r_L, 0]) cylinder(h = H, r = r);
		translate([r_l, -r_L, 0]) cylinder(h = H, r = r);

		translate([0, r_base, 0]) cylinder(h = H, r = r);
		translate([0, -r_base, 0]) cylinder(h = H, r = r);
	}
}



A=10;
L2=30;

module mount() {
	  difference() {
		union() {
			cube(size=[L2, A, H]);
			translate([L2, A, A/2]) rotate(a=90, v=[1,0,0]) cylinder(r=H/2, h=A);
		}
		translate([L2, A, A/2]) rotate(a=90, v=[1,0,0]) cylinder(r=r_bolt, h=A);
	}
};


translate([base - 2*A, l, 0]) mount();







