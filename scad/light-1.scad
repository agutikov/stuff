

module rounded_square( width, h, radius_corner )
{
    translate( [ 0, 0, 0 ] )
    minkowski() {
        square( [width - 2 * radius_corner, h - 2*radius_corner], center=true );
        circle( radius_corner );
    }
}

module rounded_piramid(x, y, h, r, scale)
{
    linear_extrude(height = h, center = false, convexity = 10, twist = 0, slices = 20, scale = scale) {
        rounded_square(x, y, r);
    }
}

module base(x, y, h1, h2)
{
    a1 = 0.98;
    rounded_piramid(x, y, h1, 10, a1);
    
    translate([0,0,h1])
        rounded_piramid(x*a1, y*a1, h2, 10, 0.2);
}

$fn=36;

module cyl (x, y, h) 
{
    linear_extrude(height = h, center = false, convexity = 10, twist = 0, slices = 20, scale = scale)
    scale([x, y, 1.0])
    circle(d=1);    
}

D1=80;
D2=70;
d=8;
h1=50;
h2=20;
a=atan((h1-h2)/D1);

Dg1=76;
Dg2=64;
hg=5;

difference() {
    union() {
        base(100, 80, 5, 20);
        
        difference() {
            cyl(D1, D2, h1);
            
            translate([0, 0, h1 - (h1-h2)/2])
            rotate([0, a, 0])
                union() {
                    translate([0,0,-hg])
                        cyl(Dg1, Dg2, hg*2);
                    translate([0, 0, h1])
                        cube([2*D1, 2*D2, 2*h1], center=true);
                }
        }
    }

    translate([0,0,-5])
        cyl(D1-2*d, D2-2*d, h1+10);
}




