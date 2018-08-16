<?php
$image1 = imagecreatefrompng('data/tsukuba_l.png'); 
$image2 = imagecreatefrompng('data/tsukuba_r.png'); 

$width = imagesx($image1);
$height = imagesy($image1);

$new_image=imagecreatetruecolor($width, $height);
//$white = imagecolorallocate($new_image, 255, 255, 255);
//$grey = imagecolorallocate($new_image, 100, 100, 100);
//$black = imagecolorallocate($new_image, 0, 0, 0);

	$startx=145;
	$starty=149;

  for ($x = 0; $x < $height; $x++) {
      for ($y = 0; $y < $width; $y++) {

         $a=imagecolorat($image1, $x, $y);
         $ar = ($a >> 16) & 0xFF;
         $ag = ($a >> 8) & 0xFF;
         $ab = $a & 0xFF;

       $b=imagecolorat($image1, $x, $y+1);
       $br = ($b >> 16) & 0xFF;
       $bg = ($b >> 8) & 0xFF;
       $bb = $b & 0xFF;

       $c=imagecolorat($image1, $x, $y+2);
       $cr = ($c >> 16) & 0xFF;
       $cg = ($c >> 8) & 0xFF;
       $cb = $c & 0xFF;

////   $d=imagecolorat($image1, $x, $y+3);
////   $dr = ($d >> 16) & 0xFF;
////   $dg = ($d >> 8) & 0xFF;
////   $db = $d & 0xFF;

////   $e=imagecolorat($image1, $x, $y+4);
////   $er = ($e >> 16) & 0xFF;
////   $eg = ($e >> 8) & 0xFF;
////   $eb = $e & 0xFF;


////   $f=imagecolorat($image1, $x, $y+5);
////   $fr = ($f >> 16) & 0xFF;
////   $fg = ($f >> 8) & 0xFF;
////   $fb = $f & 0xFF;

	  $d_smallest=10000;
	  $winning_pos;
      $color=255;
      for ($yy = $y; $yy < $width; $yy++) {
               $a2=imagecolorat($image2, $x, $yy+0);
               $ar2 = ($a2 >> 16) & 0xFF;
               $ag2 = ($a2 >> 8) & 0xFF;
               $ab2 = $a2 & 0xFF;

               $b2=imagecolorat($image2, $x, $yy+1);
               $br2 = ($b2 >> 16) & 0xFF;
               $bg2 = ($b2 >> 8) & 0xFF;
               $bb2 = $b2 & 0xFF;

               $c2=imagecolorat($image1, $x, $yy+2);
               $cr2 = ($c2 >> 16) & 0xFF;
               $cg2 = ($c2 >> 8) & 0xFF;
               $cb2 = $c2 & 0xFF;

////////       $d2=imagecolorat($image1, $x, $yy+3);
////////       $dr2 = ($d2 >> 16) & 0xFF;
////////       $dg2 = ($d2 >> 8) & 0xFF;
////////       $db2 = $d2 & 0xFF;

////////       $e2=imagecolorat($image1, $x, $yy+4);
////////       $er2 = ($e2 >> 16) & 0xFF;
////////       $eg2 = ($e2 >> 8) & 0xFF;
////////       $eb2 = $e2 & 0xFF;

////////       $f2=imagecolorat($image1, $x, $yy+5);
////////       $fr2 = ($f2 >> 16) & 0xFF;
////////       $fg2 = ($f2 >> 8) & 0xFF;
////////       $fb2 = $f2 & 0xFF;

                $d = sqrt(pow($ar-$ar2,2)+pow($ag-$ag2,2)+ pow($ab-$ab2,2))+
               sqrt(pow($br-$br2,2)+pow($bg-$bg2,2)+ pow($bb-$bb2,2))+
               sqrt(pow($cr-$cr2,2)+pow($cg-$cg2,2)+ pow($cb-$cb2,2));
            //   sqrt(pow($dr-$dr2,2)+pow($dg-$dg2,2)+ pow($db-$db2,2));
             //sqrt(pow($er-$er2,2)+pow($eg-$eg2,2)+ pow($eb-$eb2,2))+
             //sqrt(pow($fr-$fr2,2)+pow($fg-$fg2,2)+ pow($fb-$fb2,2));
                    if($d < $d_smallest){
                        $d_smallest=$d;
                        $winning_pos=$yy;
                    }
                 // if($d < 5){
                 //     echo $d ."\n";
                 // }

                    //good enough match
                    if($d <=50){
                         break;
                    }
                    //max distance y is  width
                    //further appart the closer the object
                    //we will do grey scale, closer is darker
           }
              $distance_y=abs($y-$winning_pos);
              $color= 255 - round($distance_y * .6375);
              imagesetpixel($new_image,$x,$y,imagecolorallocate($new_image, $color, $color, $color));

          }
     }
    imagejpeg($new_image, 'simpletext.jpg');
	
 ////echo $winning_pos."\n";

?>
