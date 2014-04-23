

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


