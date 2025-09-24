function y = closed_loop_observer(x_hat, xj_hat, x0_hat, yi_tilde, yj_tilde, y0_tilde ,node)

%%%%%%%%%WORKSPACE%%%%%%%%%%%%%%%%
global N n A B C D Q R P K F Adjency L G lambda c
%%%%%%%%%WORKSPACE%%%%%%%%%%%%%%%%

if node == 0
    y = [0;0];
else
    js = 1;
    epsilon = 0;
    for j=1:6
        epsilon = epsilon + Adjency(node,j)*(xj_hat(js:js+1)-x_hat);
        js = js+2;
    end
    epsilon = epsilon + G(node,node)*(x0_hat-x_hat);
    
    
    xi = 0;
    for j=1:6
        xi = xi + Adjency(node,j)*(yj_tilde(j)-yi_tilde);
    end
    xi = xi + G(node,node)*(y0_tilde-yi_tilde);
    
    y = A*x_hat + B*(c*K*epsilon) - (c*F)*xi;
end
