function y = closed_loop_dynamics(xj_hat, x_hat, x0_hat, node)

%%%%%%%%%WORKSPACE%%%%%%%%%%%%%%%%
global N n A B C D Q R P K F Adjency L G lambda c 
%%%%%%%%%WORKSPACE%%%%%%%%%%%%%%%%

if node == 0
    y = [0;0];
else
    js = 1;
    sum = 0;
    for j=1:6
        sum = sum + Adjency(node,j)*(xj_hat(js:js+1)-x_hat);
        js = js+2;
    end
    
    sum= sum + G(node,node)*(x0_hat-x_hat);
    y = (c*B*K)*sum;

end